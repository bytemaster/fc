#include <fc/thread.hpp>
#include <fc/string.hpp>
#include <fc/time.hpp>

#include <boost/thread.hpp>
#include "context.hpp"
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <vector>
#include <fc/log.hpp>

namespace fc {
    struct sleep_priority_less {
        bool operator()( const context::ptr& a, const context::ptr& b ) {
            return a->resume_time > b->resume_time;
        }
    };
    class thread_d {
        public:
           thread_d(fc::thread& s)
            :self(s), boost_thread(0),
             task_in_queue(0),
             done(false),
             current(0),
             pt_head(0),
             ready_head(0),
             ready_tail(0),
             blocked(0)
            { 
              static char cnt = 0;
              name = fc::string("th_") + char('a'+cnt); 
              cnt++;
            }
            ~thread_d(){
              slog( "...%p %s",this,name.c_str() );
            }
           fc::thread&             self;
           boost::thread* boost_thread;
           bc::stack_allocator              stack_alloc;
           boost::condition_variable        task_ready;
           boost::mutex                     task_ready_mutex;

           boost::atomic<task_base*>       task_in_queue;
           std::vector<task_base*>         task_pqueue;
           std::vector<task_base*>         task_sch_queue;
           std::vector<fc::context*>       sleep_pqueue;
           std::vector<fc::context*>       free_list;

           bool                     done;
           fc::string               name;
           fc::context*             current;

           fc::context*             pt_head;

           fc::context*             ready_head;
           fc::context*             ready_tail;

           fc::context*             blocked;



           void debug( const fc::string& s ) {
              boost::unique_lock<boost::mutex> lock(log_mutex());

              std::cerr<<"--------------------- "<<s.c_str()<<" - "<<current;
              if( current && current->cur_task ) std::cerr<<'('<<current->cur_task->get_desc()<<')';
              std::cerr<<" ---------------------------\n";
              std::cerr<<"  Ready\n";
              fc::context* c = ready_head;
              while( c ) {
                std::cerr<<"    "<<c;
                if( c->cur_task ) std::cerr<<'('<<c->cur_task->get_desc()<<')'; 
                fc::context* p = c->caller_context;
                while( p ) {
                  std::cerr<<"  ->  "<<p;
                  p = p->caller_context;
                }
                std::cerr<<"\n";
                c = c->next;
              }
              std::cerr<<"  Blocked\n";
              c = blocked;
              while( c ) {
                std::cerr<<"   ctx: "<< c; 
                if( c->cur_task ) std::cerr<<'('<<c->cur_task->get_desc()<<')'; 
                std::cerr << " blocked on prom: ";
                for( uint32_t i = 0; i < c->blocking_prom.size(); ++i ) {
                  std::cerr<<c->blocking_prom[i].prom<<'('<<c->blocking_prom[i].prom->get_desc()<<')';
                  if( i + 1 < c->blocking_prom.size() ) { 
                    std::cerr<<",";
                  }
                }

                fc::context* p = c->caller_context;
                while( p ) {
                  std::cerr<<"  ->  "<<p;
                  p = p->caller_context;
                }
                std::cerr<<"\n";
                c = c->next_blocked;
              }
              std::cerr<<"-------------------------------------------------\n";
           }

            // insert at from of blocked linked list
           inline void add_to_blocked( fc::context* c ) {
              c->next_blocked = blocked;
              blocked = c;
           }

           void pt_push_back(fc::context* c) {
              c->next = pt_head;
              pt_head = c;
              /* 
              fc::context* n = pt_head;
              int i = 0;
              while( n ) {
                ++i;
                n = n->next;
              }
              wlog( "idle context...%2%  %1%", c, i );
              */
           }
           fc::context::ptr ready_pop_front() {
                fc::context::ptr tmp = 0;
                if( ready_head ) {
                    tmp        = ready_head;
                    ready_head = tmp->next;
                    if( !ready_head )   
                        ready_tail = 0;
                    tmp->next = 0;
                }
                return tmp;
           }
           void ready_push_front( const fc::context::ptr& c ) {
           //     c->ready_time = time_point::now();
                c->next = ready_head;
                ready_head = c;
                if( !ready_tail ) 
                    ready_tail = c;
           }
           void ready_push_back( const fc::context::ptr& c ) {
            //    c->ready_time = time_point::now();
                c->next = 0;
                if( ready_tail ) { 
                    ready_tail->next = c;
                } else {
                    ready_head = c;
                }
                ready_tail = c;
           }
           struct task_priority_less {
               bool operator()( task_base* a, task_base* b ) {
                   return a->_prio.value < b->_prio.value ? true :  (a->_prio.value > b->_prio.value ? false : a->_posted_num > b->_posted_num );
               }
           };
           struct task_when_less {
                bool operator()( task_base* a, task_base* b ) {
                    return a->_when > b->_when;
                }
           };

           void enqueue( task_base* t ) {
                time_point now = time_point::now();
                task_base* cur = t;
                while( cur ) {
                  if( cur->_when > now ) {
                    task_sch_queue.push_back(cur);
                    std::push_heap( task_sch_queue.begin(),
                                    task_sch_queue.end(), task_when_less()   );
                  } else {
                    task_pqueue.push_back(cur);
                    BOOST_ASSERT( this == thread::current().my );
                    std::push_heap( task_pqueue.begin(),
                                    task_pqueue.end(), task_priority_less()   );
                  }
                    cur = cur->_next;
                }
           }
           task_base* dequeue() {
                // get a new task
                BOOST_ASSERT( this == thread::current().my );
                
                task_base* pending = 0; 

                pending = task_in_queue.exchange(0,boost::memory_order_consume);
                if( pending ) { enqueue( pending ); }

                task_base* p(0);
                if( task_sch_queue.size() ) {
                    if( task_sch_queue.front()->_when <= time_point::now() ) {
                        p = task_sch_queue.front();
                        std::pop_heap(task_sch_queue.begin(), task_sch_queue.end(), task_when_less() );
                        task_sch_queue.pop_back();
                        return p;
                    }
                }
                if( task_pqueue.size() ) {
                    p = task_pqueue.front();
                    std::pop_heap(task_pqueue.begin(), task_pqueue.end(), task_priority_less() );
                    task_pqueue.pop_back();
                }
                return p;
           }
           
           /**
            * This should be before or after a context switch to
            * detect quit/cancel operations and throw an exception.
            */
           void check_fiber_exceptions() {
              if( current && current->canceled ) {
                BOOST_THROW_EXCEPTION( task_canceled() );
              } else if( done )  {
                BOOST_THROW_EXCEPTION( thread_quit() );
              }
           }
           
           /**
            *   Find the next available context and switch to it.
            *   If none are available then create a new context and
            *   have it wait for something to do.
            */
           bool start_next_fiber( bool reschedule = false ) {
              check_for_timeouts();
              if( !current ) current = new fc::context( &fc::thread::current() );

              // check to see if any other contexts are ready
              if( ready_head ) { 
                fc::context* next = ready_pop_front();
                BOOST_ASSERT( next != current ); 
                if( reschedule ) ready_push_back(current);

                // jump to next context, saving current context
                fc::context* prev = current;
                current = next;
                bc::jump_fcontext( &prev->my_context, &next->my_context, 0 );
                current = prev;
                BOOST_ASSERT( current );
              } else { // all contexts are blocked, create a new context 
                       // that will process posted tasks...
                if( reschedule )  ready_push_back(current);
                
                fc::context* next;
                if( pt_head ) {
                  next = pt_head;
                  pt_head = pt_head->next;
                  next->next = 0;
                } else {
                  next = new fc::context( &thread_d::start_process_tasks, stack_alloc,
                                                                      &fc::thread::current() );
                }
                fc::context* prev = current;
                current = next;
                bc::jump_fcontext( &prev->my_context, &next->my_context, (intptr_t)this );
                current = prev;
                BOOST_ASSERT( current );
              }

              if( current->canceled )
                  BOOST_THROW_EXCEPTION( task_canceled() );

              return true;
           }

           static void start_process_tasks( intptr_t my ) {
              thread_d* self = (thread_d*)my;
              try {
                self->process_tasks();
              } catch ( ... ) {
                std::cerr<<"fiber exited with uncaught exception:\n "<< 
                      boost::current_exception_diagnostic_information() <<std::endl;
              }
              self->free_list.push_back(self->current);
              self->start_next_fiber( false );
           }

           bool run_next_task() {
                check_for_timeouts();
                task_base* next = dequeue();
                if( next ) {
                    next->_set_active_context( current );
                    current->cur_task = next;
                    next->run();
                    current->cur_task = 0;
                    next->_set_active_context(0);
                    next->release();
                    return true;
                }
                return false;
           }
           bool has_next_task() {
             if( task_pqueue.size() ||
                 (task_sch_queue.size() && task_sch_queue.front()->_when <= time_point::now()) ||
                 task_in_queue.load( boost::memory_order_relaxed ) )
                  return true;
             return false;
           }
           void clear_free_list() {
              for( uint32_t i = 0; i < free_list.size(); ++i ) {
                delete free_list[i];
              }
              free_list.clear();
           }
           void process_tasks() {
              while( !done || blocked ) {
                if( run_next_task() ) continue;

                // if I have something else to do other than
                // process tasks... do it.
                if( ready_head ) { 
                   pt_push_back( current ); 
                   start_next_fiber(false);  
                   continue;
                }

                clear_free_list();

                { // lock scope
                  boost::unique_lock<boost::mutex> lock(task_ready_mutex);
                  if( has_next_task() ) continue;
                  time_point timeout_time = check_for_timeouts();
                  
                  if( done ) return;
                  if( timeout_time == time_point::max() ) {
                    task_ready.wait( lock );
                  } else if( timeout_time != time_point::min() ) {
                    task_ready.wait_until( lock, boost::chrono::system_clock::time_point() + 
                                                 boost::chrono::microseconds(timeout_time.time_since_epoch().count()) );
                  }
                }
              }
           }
    /**
     *    Return system_clock::time_point::min() if tasks have timed out
     *    Retunn system_clock::time_point::max() if there are no scheduled tasks
     *    Return the time the next task needs to be run if there is anything scheduled.
     */
    time_point check_for_timeouts() {
        if( !sleep_pqueue.size() && !task_sch_queue.size() ) {
            return time_point::max();
        }


        time_point next = time_point::max();
        if( task_sch_queue.size() && next > task_sch_queue.front()->_when )
          next = task_sch_queue.front()->_when;
        if( sleep_pqueue.size() && next > sleep_pqueue.front()->resume_time )
          next = sleep_pqueue.front()->resume_time;

        time_point now = time_point::now();
        if( now < next ) { return next; }

        // move all expired sleeping tasks to the ready queue
        while( sleep_pqueue.size() && sleep_pqueue.front()->resume_time < now ) {
            fc::context::ptr c = sleep_pqueue.front();
            std::pop_heap(sleep_pqueue.begin(), sleep_pqueue.end(), sleep_priority_less() );
            sleep_pqueue.pop_back();

            if( c->blocking_prom.size() ) {
                c->timeout_blocking_promises();
            }
            else { ready_push_front( c ); }
        }
        return time_point::min();
    }

    void unblock( fc::context* c ) {
      if(  fc::thread::current().my != this ) {
        async( [=](){ unblock(c); } );
        return;
      }
      ready_push_front(c); 
    }

        void yield_until( const time_point& tp, bool reschedule ) {
          check_fiber_exceptions();

          if( tp <= time_point::now() ) 
            return;

          if( !current )  {
            current = new fc::context(&fc::thread::current());
          }

          current->resume_time = tp;
          current->clear_blocking_promises();

          sleep_pqueue.push_back(current);
          std::push_heap( sleep_pqueue.begin(),
                          sleep_pqueue.end(), sleep_priority_less()   );

          start_next_fiber(reschedule);

          // clear current context from sleep queue...
          for( uint32_t i = 0; i < sleep_pqueue.size(); ++i ) {
            if( sleep_pqueue[i] == current ) {
              sleep_pqueue[i] = sleep_pqueue.back();
              sleep_pqueue.pop_back();
              std::make_heap( sleep_pqueue.begin(),
                              sleep_pqueue.end(), sleep_priority_less() );
              break;
            }
          }

          current->resume_time = time_point::max();
          check_fiber_exceptions();
        }

        void wait( const promise_base::ptr& p, const time_point& timeout ) {
          if( p->ready() ) return;
          if( timeout < time_point::now() ) 
              BOOST_THROW_EXCEPTION( future_wait_timeout() );
          
          if( !current ) { 
            current = new fc::context(&fc::thread::current()); 
          }
          
          //slog( "                                 %1% blocking on %2%", current, p.get() );
          current->add_blocking_promise(p.get(),true);

          // if not max timeout, added to sleep pqueue
          if( timeout != time_point::max() ) {
              current->resume_time = timeout;
              sleep_pqueue.push_back(current);
              std::push_heap( sleep_pqueue.begin(),
                              sleep_pqueue.end(), 
                              sleep_priority_less()   );
          }

        //  elog( "blocking %1%", current );
          add_to_blocked( current );
       //   debug("swtiching fibers..." );


          start_next_fiber();
         // slog( "resuming %1%", current );

          //slog( "                                 %1% unblocking blocking on %2%", current, p.get() );
          current->remove_blocking_promise(p.get());

          check_fiber_exceptions();
        }
    };
} // namespace fc
