#include <fc/thread/task.hpp>
#include <fc/exception/exception.hpp>
#include <fc/thread/unique_lock.hpp>
#include <fc/thread/spin_lock.hpp>
#include <fc/fwd_impl.hpp>
#include "context.hpp"

#include <fc/log/logger.hpp>
#include <boost/exception/all.hpp>

#ifdef _MSC_VER
# include <fc/thread/thread.hpp>
# include <Windows.h>
#endif

namespace fc {
  task_base::task_base(void* func)
  :
  promise_base("task_base"),
  _posted_num(0),
  _active_context(nullptr),
  _next(nullptr),
  _task_specific_data(nullptr),
  _promise_impl(nullptr),
  _functor(func){
  }

  void task_base::run() {
#ifdef _MSC_VER
    __try {
#endif
      run_impl();
#ifdef _MSC_VER
    } __except (get_unhandled_structured_exception_filter() ? get_unhandled_structured_exception_filter()(GetExceptionCode(), GetExceptionInformation()) : EXCEPTION_CONTINUE_SEARCH) {
       ExitProcess(1);
    }
#endif
  }

  void task_base::run_impl() {
    try {
      if( !canceled() )
         _run_functor( _functor, _promise_impl  );
      else
         FC_THROW_EXCEPTION( canceled_exception, "${description}", ("description", get_desc() ) );
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

  void task_base::cancel(const char* reason /* = nullptr */)
  {
    promise_base::cancel(reason);
    if (_active_context)
    {
      _active_context->canceled = true;
#ifndef NDEBUG
      _active_context->cancellation_reason = reason;
#endif
    }
  }

  task_base::~task_base() {
    cleanup_task_specific_data();
    _destroy_functor( _functor );
  }

  void   task_base::_set_active_context(context* c) {
      { synchronized( *_spinlock )
        _active_context = c; 
      }
  }

  void task_base::cleanup_task_specific_data()
  {
    if (_task_specific_data)
    {
      for (auto iter = _task_specific_data->begin(); iter != _task_specific_data->end(); ++iter)
        if (iter->cleanup)
            iter->cleanup(iter->value);
      delete _task_specific_data;
      _task_specific_data = nullptr;
    }
  }

}
