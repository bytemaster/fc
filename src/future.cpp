#include <fc/future.hpp>
#include <fc/spin_yield_lock.hpp>
#include <fc/exception.hpp>
#include <fc/thread.hpp>
#include <fc/error.hpp>
#include <fc/unique_lock.hpp>

#include <boost/assert.hpp>


namespace fc {

  promise_base::promise_base( const char* desc )
  :_ready(false),
   _blocked_thread(nullptr),
   _timeout(time_point::max()),
   _canceled(false),
   _desc(desc),
   _compl(nullptr)
  { }

  const char* promise_base::get_desc()const{
    return _desc; 
  }
               
  void promise_base::cancel(){
    _canceled = true;
  }
  bool promise_base::ready()const {
    return _ready;
  }
  bool promise_base::error()const {
    { synchronized(_spin_yield) 
      return _exceptp;
    }
  }

  void promise_base::set_exception( const fc::exception_ptr& e ){
    _exceptp = e;
    _set_value(nullptr);
  }

  void promise_base::_wait( const microseconds& timeout_us ){
     if( timeout_us == microseconds::max() ) _wait_until( time_point::max() );
     else _wait_until( time_point::now() + timeout_us );
  }
  void promise_base::_wait_until( const time_point& timeout_us ){
    { synchronized(_spin_yield) 
      if( _ready ) {
        if( _exceptp ) fc::rethrow_exception( _exceptp );
        return;
      }
      _enqueue_thread();
    }
    thread::current().wait_until( ptr(this,true), timeout_us );
    if( _ready ) {
       if( _exceptp ) fc::rethrow_exception( _exceptp );
       return; 
    }
    FC_THROW( future_wait_timeout() );
  }
  void promise_base::_enqueue_thread(){
     _blocked_thread =&thread::current();
  }
  void promise_base::_notify(){
    if( _blocked_thread != nullptr ) 
      _blocked_thread->notify(ptr(this,true));
  }
  promise_base::~promise_base() { }
  void promise_base::_set_timeout(){
    if( _ready ) 
      return;
    set_exception( fc::copy_exception( future_wait_timeout() ) );
  }
  void promise_base::_set_value(const void* s){
 //   slog( "%p == %d", &_ready, int(_ready));
//    BOOST_ASSERT( !_ready );
    { synchronized(_spin_yield) 
      _ready = true;
    }
    _notify();
    if( nullptr != _compl ) {
      _compl->on_complete(s,_exceptp);
    }
  }
  void promise_base::_on_complete( detail::completion_handler* c ) {
    { synchronized(_spin_yield) 
        delete _compl; 
        _compl = c;
    }
  }
}

