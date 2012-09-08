#ifndef _FC_TASK_HPP_
#define _FC_TASK_HPP_
#include <fc/future.hpp>
#include <fc/priority.hpp>

namespace fc {
  struct context;

  class task_base : virtual public promise_base {
    public:
      ~task_base();
      void        run(); 
    protected:
      uint64_t    _posted_num;
      priority    _prio;
      time_point  _when;
      void        _set_active_context(context*);
      context*    _active_context;
      task_base*  _next;

      task_base(void* func);
      // opaque internal / private data used by
      // thread/thread_private
      friend class thread;
      friend class thread_d;
      char          _spinlock_store[sizeof(void*)];

      // avoid rtti info for every possible functor...
      promise_base* _promise_impl;
      void*         _functor;
      void          (*_destroy_functor)(void*);
      void          (*_run_functor)(void*, void* );
  };

  namespace detail {
    template<typename T>
    struct functor_destructor {
      static void destroy( void* v ) { ((T*)v)->~T(); }
    };
    template<typename T>
    struct functor_run {
      static void run( void* functor, void* prom ) {
        ((promise<decltype((*((T*)functor))())>*)prom)->set_value( (*((T*)functor))() );
      }
    };
    template<typename T>
    struct void_functor_run {
      static void run( void* functor, void* prom ) {
        (*((T*)functor))();
        ((promise<void>*)prom)->set_value();
      }
    };
  }

  template<typename R,uint64_t FunctorSize=64>
  class task : virtual public task_base, virtual public promise<R> {
    public:
      template<typename Functor>
      task( Functor&& f ):task_base(&_functor[0]) {
        static_assert( sizeof(f) <= sizeof(_functor), "sizeof(Functor) is larger than FunctorSize" );
        new ((char*)&_functor[0]) Functor( fc::forward<Functor>(f) );
        _destroy_functor = &detail::functor_destructor<Functor>::destroy;

        _promise_impl = static_cast<promise<R>*>(this);
        _run_functor  = &detail::functor_run<Functor>::run;
      }
      char _functor[FunctorSize];
  };
  template<uint64_t FunctorSize>
  class task<void,FunctorSize> : virtual public task_base, virtual public promise<void> {
    public:
      template<typename Functor>
      task( Functor&& f ):task_base(&_functor[0]) {
        static_assert( sizeof(f) <= sizeof(_functor), "sizeof(Functor) is larger than FunctorSize"  );
        new ((char*)&_functor[0]) Functor( fc::forward<Functor>(f) );
        _destroy_functor = &detail::functor_destructor<Functor>::destroy;

        _promise_impl = static_cast<promise<void>*>(this);
        _run_functor  = &detail::void_functor_run<Functor>::run;
      }
      char _functor[FunctorSize];
  };

}

#endif // _FC_TASK_HPP_
