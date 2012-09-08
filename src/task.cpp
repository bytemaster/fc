#include <fc/task.hpp>
#include <fc/exception.hpp>
#include <fc/unique_lock.hpp>
#include <fc/spin_lock.hpp>

namespace fc {
  task_base::task_base(void* func)
  :_functor(func){
    new (&_spinlock_store[0]) fc::spin_lock();
  }

  void task_base::run() {
    try {
      _run_functor( _functor, _promise_impl  );
    } catch ( ... ) {
      _promise_impl->set_exception( current_exception() );
    }
  }
  task_base::~task_base() {
    _destroy_functor( _functor );
  }

  void   task_base::_set_active_context(context* c) {
      void* p = &_spinlock_store[0];
      { synchronized( *((fc::spin_lock*)p)) 
        _active_context = c; 
      }
  }
}
