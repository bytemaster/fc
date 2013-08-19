#include <fc/thread/thread.hpp>
#include <fc/vector.hpp>
#include <fc/io/sstream.hpp>
#include <fc/log/logger.hpp>
#include "thread_d.hpp"

namespace fc {
  const char* thread_name() {
    return thread::current().name().c_str();
  }
  void* thread_ptr() {
    return &thread::current();
  }

   thread*& current_thread() {
      #ifdef _MSC_VER
         static __declspec(thread) thread* t = NULL;
      #else
         static __thread thread* t = NULL;
      #endif
      return t;
   }

   thread::thread( const char* name  ) {
      promise<void>::ptr p(new promise<void>());
      boost::thread* t = new boost::thread( [this,p,name]() {
          try {
            this->my = new thread_d(*this);
            current_thread() = this;
            p->set_value();
            exec();
          } catch ( fc::exception& e ) {
            wlog( "unhandled exception" );
            p->set_exception( e.dynamic_copy_exception() );
          } catch ( ... ) {
            wlog( "unhandled exception" );
            p->set_exception( std::make_shared<unhandled_exception>( FC_LOG_MESSAGE( warn, "unhandled exception: ${diagnostic}", ("diagnostic",boost::current_exception_diagnostic_information()) ) ) );
            //assert( !"unhandled exception" );
            //elog( "Caught unhandled exception %s", boost::current_exception_diagnostic_information().c_str() );
          }
      } );
      p->wait();
      my->boost_thread = t;
      set_name(name);
   }
   thread::thread( thread_d* ) {
     my = new thread_d(*this);
   }

   thread::thread( thread&& m ) {
    my = m.my;
    m.my = 0;
   }

   thread& thread::operator=(thread&& t ) {
      fc_swap(t.my,my);
      return *this;
   }

   thread::~thread() {
      //slog( "my %p", my );
      if( is_current() )
      {
        wlog( "delete my" );
        delete my;
      }
      my = 0;
   }

   thread& thread::current() {
     if( !current_thread() ) current_thread() = new thread((thread_d*)0);
     return *current_thread();
   }
   const string& thread::name()const { return my->name; }
   void          thread::set_name( const fc::string& n ) { my->name = n; }
   void          thread::debug( const fc::string& d ) { /*my->debug(d);*/ }

   void thread::quit() {
     //if quiting from a different thread, start quit task on thread.
     //If we have and know our attached boost thread, wait for it to finish, then return.
      if( &current() != this ) {
          async( [=](){quit();} );//.wait();
          if( my->boost_thread ) {
            auto n = name();
            ilog( "joining... ${n}", ("n",n) );//n.c_str() );
            my->boost_thread->join();
            delete my;
            my = nullptr;
            ilog( "done joining...${n}", ("n",n) ); //n.c_str() );
          }
          return;
      }

 //     wlog( "%s", my->name.c_str() );
      // We are quiting from our own thread...

      // break all promises, thread quit!
      while( my->blocked ) {
        fc::context* cur  = my->blocked;
        while( cur ) {
            fc::context* n = cur->next;
            // this will move the context into the ready list.
            //cur->prom->set_exception( boost::copy_exception( error::thread_quit() ) );
            //cur->except_blocking_promises( thread_quit() );
            cur->except_blocking_promises( std::make_shared<canceled_exception>() );
               
            cur = n;
        }
        if( my->blocked ) { 
  //        wlog( "still blocking... whats up with that?");
  //        debug( "on quit" ); 
        }
      }
      BOOST_ASSERT( my->blocked == 0 );
      //my->blocked = 0;
      
      
      // move all sleep tasks to ready
      for( uint32_t i = 0; i < my->sleep_pqueue.size(); ++i ) {
        my->ready_push_front( my->sleep_pqueue[i] );
      }
      my->sleep_pqueue.clear();

      // move all idle tasks to ready
      fc::context* cur = my->pt_head;
      while( cur ) {
        fc::context* n = cur->next;
        cur->next = 0;
        my->ready_push_front( cur );
        cur = n;
      }

      // mark all ready tasks (should be everyone)... as canceled 
      cur = my->ready_head;
      while( cur ) {
        cur->canceled = true;
        cur = cur->next;
      }
      my->done = true;

      // now that we have poked all fibers... switch to the next one and
      // let them all quit.
      while( my->ready_head ) { 
        my->start_next_fiber(true); 
        my->check_for_timeouts();
      }
      my->clear_free_list();
   }
     
   void thread::exec() {
      if( !my->current ) my->current = new fc::context(&fc::thread::current());
      try {
      my->process_tasks(); 
      } 
      catch( canceled_exception& )
      {
         wlog( "thread canceled" );
      }
      delete my->current;
      my->current = 0;
   }
     
   bool thread::is_running()const {
      return !my->done;
   }
     
   priority thread::current_priority()const {
      BOOST_ASSERT(my);
      if( my->current ) return my->current->prio;
      return priority();
   }

   void thread::yield(bool reschedule ) {
      my->check_fiber_exceptions();
      my->start_next_fiber(reschedule);
      my->check_fiber_exceptions();
   }
   void thread::sleep_until( const time_point& tp ) {
      //ilog( "sleep until ${tp}    wait: ${delta}", ("tp",tp)("delta",(tp-fc::time_point::now()).count()) );
     
      if( tp <= (time_point::now()+fc::microseconds(500)) ) 
      {
         this->yield(true);
      }
      my->yield_until( tp, false );
      /*
      my->check_fiber_exceptions();
      
      BOOST_ASSERT( &current() == this );
      if( !my->current )  {
        my->current = new fc::context(&fc::thread::current());
      }

      my->current->resume_time = tp;
      my->current->clear_blocking_promises();

      my->sleep_pqueue.push_back(my->current);
      std::push_heap( my->sleep_pqueue.begin(),
                      my->sleep_pqueue.end(), sleep_priority_less()   );

      my->start_next_fiber();
      my->current->resume_time = time_point::maximum();

      my->check_fiber_exceptions();
      */
   }
   int  thread::wait_any_until( std::vector<promise_base::ptr>&& p, const time_point& timeout) {
       for( size_t i = 0; i < p.size(); ++i ) {
         if( p[i]->ready() ) return i;
       }

       if( timeout < time_point::now() ) {
           fc::stringstream ss;
           for( auto i = p.begin(); i != p.end(); ++i ) {
            ss << (*i)->get_desc() <<", ";
           }
           FC_THROW_EXCEPTION( timeout_exception, "${task}", ("task",ss.str()) );
       }
       
       if( !my->current ) { 
         my->current = new fc::context(&fc::thread::current()); 
       }
     
       for( uint32_t i = 0; i < p.size(); ++i ) {
           my->current->add_blocking_promise(p[i].get(),false);
       };

       // if not max timeout, added to sleep pqueue
       if( timeout != time_point::maximum() ) {
           my->current->resume_time = timeout;
           my->sleep_pqueue.push_back(my->current);
           std::push_heap( my->sleep_pqueue.begin(),
                           my->sleep_pqueue.end(), 
                           sleep_priority_less()   );
       }
       my->add_to_blocked( my->current );
       my->start_next_fiber();

       for( auto i = p.begin(); i != p.end(); ++i ) {
           my->current->remove_blocking_promise(i->get());
       }
     
       my->check_fiber_exceptions();

       for( uint32_t i = 0; i < p.size(); ++i ) {
         if( p[i]->ready() ) return i;
       }
       //BOOST_THROW_EXCEPTION( wait_any_error() );
       return -1;
   }

   void thread::async_task( task_base* t, const priority& p, const char* desc ) {
      async_task( t, p, time_point::min(), desc );
   }

   void thread::poke() {
     boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
     my->task_ready.notify_one();
   }

   void thread::async_task( task_base* t, const priority& p, const time_point& tp, const char* desc ) {
      assert(my);
      t->_when = tp;
     // slog( "when %lld", t->_when.time_since_epoch().count() );
     // slog( "delay %lld", (tp - fc::time_point::now()).count() );
      task_base* stale_head = my->task_in_queue.load(boost::memory_order_relaxed);
      do { t->_next = stale_head;
      }while( !my->task_in_queue.compare_exchange_weak( stale_head, t, boost::memory_order_release ) );

      // Because only one thread can post the 'first task', only that thread will attempt
      // to aquire the lock and therefore there should be no contention on this lock except
      // when *this thread is about to block on a wait condition.  
      if( this != &current() &&  !stale_head ) { 
          boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
          my->task_ready.notify_one();
      }
   }

   void yield() {
      thread::current().yield();
   }
   void usleep( const microseconds& u ) {
      thread::current().sleep_until( time_point::now() + u);
   }
   void sleep_until( const time_point& tp ) {
      thread::current().sleep_until(tp);
   }

   void  exec() {
      return thread::current().exec();
   }

   int wait_any( std::vector<promise_base::ptr>&& v, const microseconds& timeout_us  ) {
      return thread::current().wait_any_until( fc::move(v), time_point::now() + timeout_us );
   }
   int wait_any_until( std::vector<promise_base::ptr>&& v, const time_point& tp ) {
      return thread::current().wait_any_until( fc::move(v), tp );
   }
   void thread::wait_until( promise_base::ptr&& p, const time_point& timeout ) {
         if( p->ready() ) return;
         if( timeout < time_point::now() ) 
             FC_THROW_EXCEPTION( timeout_exception, "${task}", ("task", p->get_desc()) );
         
         if( !my->current ) { 
           my->current = new fc::context(&fc::thread::current()); 
         }
         
         //slog( "                                 %1% blocking on %2%", my->current, p.get() );
         my->current->add_blocking_promise(p.get(),true);

         // if not max timeout, added to sleep pqueue
         if( timeout != time_point::maximum() ) {
             my->current->resume_time = timeout;
             my->sleep_pqueue.push_back(my->current);
             std::push_heap( my->sleep_pqueue.begin(),
                             my->sleep_pqueue.end(), 
                             sleep_priority_less()   );
         }

       //  elog( "blocking %1%", my->current );
         my->add_to_blocked( my->current );
      //   my->debug("swtiching fibers..." );


         my->start_next_fiber();
        // slog( "resuming %1%", my->current );

         //slog( "                                 %1% unblocking blocking on %2%", my->current, p.get() );
         my->current->remove_blocking_promise(p.get());

         my->check_fiber_exceptions();
    }

    void thread::notify( const promise_base::ptr& p ) {
      //slog( "this %p  my %p", this, my );
      BOOST_ASSERT(p->ready());
      if( !is_current() ) {
        this->async( [=](){ notify(p); }, "notify", priority::max() );
        return;
      }
      // TODO: store a list of blocked contexts with the promise 
      //  to accelerate the lookup.... unless it introduces contention...
      
      // iterate over all blocked contexts


      fc::context* cur_blocked  = my->blocked;
      fc::context* prev_blocked = 0;
      while( cur_blocked ) {
        // if the blocked context is waiting on this promise 
        if( cur_blocked->try_unblock( p.get() )  ) {
          // remove it from the blocked list.

          // remove this context from the sleep queue...
          for( uint32_t i = 0; i < my->sleep_pqueue.size(); ++i ) {
            if( my->sleep_pqueue[i] == cur_blocked ) {
              my->sleep_pqueue[i]->blocking_prom.clear();
              my->sleep_pqueue[i] = my->sleep_pqueue.back();
              my->sleep_pqueue.pop_back();
              std::make_heap( my->sleep_pqueue.begin(),my->sleep_pqueue.end(), sleep_priority_less() );
              break;
            }
          }
          auto cur = cur_blocked;
          if( prev_blocked ) {  
              prev_blocked->next_blocked = cur_blocked->next_blocked; 
              cur_blocked =  prev_blocked->next_blocked;
          } else { 
              my->blocked = cur_blocked->next_blocked; 
              cur_blocked = my->blocked;
          }
          cur->next_blocked = 0;
          my->ready_push_front( cur );
        } else { // goto the next blocked task
          prev_blocked  = cur_blocked;
          cur_blocked   = cur_blocked->next_blocked;
        }
      }
    }
    bool thread::is_current()const {
      return this == &current();
    }


}
