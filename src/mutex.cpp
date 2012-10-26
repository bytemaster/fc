#include <fc/mutex.hpp>
#include <fc/thread.hpp>
#include <fc/unique_lock.hpp>
#include <fc/log.hpp>
#include "context.hpp"
#include "thread_d.hpp"

namespace fc {

  mutex::mutex()
  :m_blist(0){}

  mutex::~mutex() {
    if( m_blist ) {
      auto c = m_blist;
      fc::thread::current().debug("~mutex");
      while( c )  {
        elog( "still blocking on context %p (%s)", m_blist, (m_blist->cur_task ? m_blist->cur_task->get_desc() : "no current task") ); 
        c = c->next_blocked;
      }
    }
    BOOST_ASSERT( !m_blist && "Attempt to free mutex while others are blocking on lock." );
  }

  /**
   *  @param  next - is set to the next context to get the lock.
   *  @return the last context (the one with the lock)
   */
  static fc::context* get_tail( fc::context* h, fc::context*& next ) {
    next = 0;
    fc::context* n = h;
    if( !n ) return n;
    while( n->next_blocked ) { 
      next = n;
      n=n->next_blocked;
    }
    return n;
  }
  static fc::context* remove( fc::context* head, fc::context* target ) {
    fc::context* c = head;
    fc::context* p = 0;
    while( c ) {
      if( c == target ) {
        if( p ) { 
          p->next_blocked = c->next_blocked; 
          return head; 
        }
        return c->next_blocked;
      }
      p = c;
      c = c->next_blocked;
    }
    return head;
  }
  static void cleanup( fc::mutex& m, fc::spin_yield_lock& syl, fc::context*& bl, fc::context* cc ) {
    {  
      fc::unique_lock<fc::spin_yield_lock> lock(syl);
      if( cc->next_blocked ) {
        bl = remove(bl, cc ); 
        return;
      }
    }
    m.unlock();
  }

  /**
   *  A mutex is considered to hold the lock when
   *  the current context is the tail in the wait queue.
   */
  bool mutex::try_lock() {
    fc::thread* ct = &fc::thread::current();
    fc::context* cc = ct->my->current;
    fc::context* n  = 0;

    fc::unique_lock<fc::spin_yield_lock> lock(m_blist_lock, fc::try_to_lock_t());
    if( !lock  ) 
      return false;

    if( !m_blist ) { 
      m_blist = cc;
      return true;
    }
    // allow recursive locks.
    return ( get_tail( m_blist, n ) == cc );
  }

  bool mutex::try_lock_until( const fc::time_point& abs_time ) {
    fc::context* n  = 0;
    fc::context* cc = fc::thread::current().my->current;

    { // lock scope
      fc::unique_lock<fc::spin_yield_lock> lock(m_blist_lock,abs_time);
      if( !lock ) return false;

      if( !m_blist ) { 
        m_blist = cc;
        return true;
      }

      // allow recusive locks
      if ( get_tail( m_blist, n ) == cc ) 
        return true;

      cc->next_blocked = m_blist;
      m_blist = cc;
    } // end lock scope
    try {
        fc::thread::current().my->yield_until( abs_time, false );
        return( 0 == cc->next_blocked );
    } catch (...) {
      cleanup( *this, m_blist_lock, m_blist, cc);
      throw;
    }
  }

  void mutex::lock() {
    fc::context* n  = 0;
    fc::context* cc = fc::thread::current().my->current;
    {
      fc::unique_lock<fc::spin_yield_lock> lock(m_blist_lock);
      if( !m_blist ) { 
        m_blist = cc;
        return;
      }

      // allow recusive locks
      if ( get_tail( m_blist, n ) == cc ) {
        return;
      }
      cc->next_blocked = m_blist;
      m_blist = cc;

      int cnt = 0;
      auto i = m_blist;
      while( i ) {
        i = i->next_blocked;
        ++cnt;
      }
      wlog( "wait queue len %1%", cnt );
    }

    try {
      fc::thread::current().yield(false);
      BOOST_ASSERT( cc->next_blocked == 0 );
    } catch ( ... ) {
      wlog( "lock with throw %p %s",this, fc::current_exception().diagnostic_information().c_str() );
      cleanup( *this, m_blist_lock, m_blist, cc);
      throw;
    }
  }

  void mutex::unlock() {
    fc::context* next = 0;
    { fc::unique_lock<fc::spin_yield_lock> lock(m_blist_lock);
      get_tail(m_blist, next);
      if( next ) {
        next->next_blocked = 0;
        next->ctx_thread->my->unblock( next );
      } else {
        m_blist   = 0;
      }
    }
  }

} // fc


