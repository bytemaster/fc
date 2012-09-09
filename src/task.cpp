#include <fc/task.hpp>
#include <fc/exception.hpp>
#include <fc/unique_lock.hpp>
#include <fc/spin_lock.hpp>
#include <fc/fwd_impl.hpp>


namespace fc {
  task_base::task_base(void* func)
  :_functor(func){
  }

  void task_base::run() {
    try {
      _run_functor( _functor, _promise_impl  );
    } catch ( ... ) {
      set_exception( current_exception() );
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
