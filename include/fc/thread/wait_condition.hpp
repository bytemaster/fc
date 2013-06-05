#pragma once
#include <fc/thread/future.hpp>
#include <fc/thread/spin_yield_lock.hpp>
#include <fc/thread/unique_lock.hpp>
#include <deque>

namespace fc
{
  /**
   *  A thread-safe, fiber-aware condition variable that
   *  can be used to signal/wait on a certain condition between
   *  threads.
   */
  template<typename T=void_t>
  class wait_condition
  {
     public:
        void wait( const microseconds& timeout = microseconds::maximum() )
        {
          typename fc::promise<T>::ptr p = new fc::promise<T>();
          { synchronized( _prom_lock ) 
            _promises.push_back( p );
          }
          p->wait(timeout);
        }

        template<typename LockType>
        T wait( LockType& l, const microseconds& timeout = microseconds::maximum() )
        {
          typename fc::promise<T>::ptr p( new fc::promise<T>());
          { synchronized( _prom_lock ) 
            _promises.push_back( p );
          }
          l.unlock();
          return p->wait(timeout);
        }

        void notify_one( const T& t=T())
        {
          typename fc::promise<void_t>::ptr prom;
          { synchronized( _prom_lock ) 
             if( _promises.size() )
             {
                prom = _promises.front();
                _promises.pop_front();
             }
          }

          if( prom && prom->retain_count() > 1 ) 
             prom->set_value(t);
        }
        void notify_all(const T& t=T())
        {
            std::deque<typename fc::promise<T>::ptr> all;
            { synchronized( _prom_lock ) 
              all = fc::move(_promises);
            }
            for( auto itr = all.begin(); itr != all.end(); ++itr )
            {
              if( (*itr)->retain_count() > 1 ) 
                 (*itr)->set_value(t);
            }
        }
     private:
        fc::spin_yield_lock                      _prom_lock;
        std::deque<typename fc::promise<T>::ptr> _promises;
  };
}
