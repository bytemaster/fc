#include <fc/thread/task.hpp>
#include <fc/exception/exception.hpp>
#include <fc/thread/unique_lock.hpp>
#include <fc/thread/spin_lock.hpp>
#include <fc/fwd_impl.hpp>

#include <fc/log/logger.hpp>
#include <boost/exception/all.hpp>

namespace fc {
  task_base::task_base(void* func)
  :_functor(func){
  }

  void task_base::run() {
    try {
      _run_functor( _functor, _promise_impl  );
    } 
    catch ( const exception& e ) 
    {
      set_exception( e.dynamic_copy_exception() );
    } 
    catch ( ... ) 
    {
       set_exception( std::make_shared<unhandled_exception>( FC_LOG_MESSAGE( warn, "unhandled exception: ${diagnostic}", ("diagnostic",boost::current_exception_diagnostic_information()) ) ) );
    }
  }
  task_base::~task_base() {
    _destroy_functor( _functor );
  }

  void   task_base::_set_active_context(context* c) {
      { synchronized( *_spinlock )
        _active_context = c; 
      }
  }
}
