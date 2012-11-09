#pragma once
#include <fc/ptr.hpp>
#include <fc/thread.hpp>

namespace fc {

  namespace detail {
    struct actor_member { 
        // TODO: expand for all method arity and constness....
        template<typename R, typename C, typename A1, typename P>
        static std::function<fc::future<R>(A1)> functor( P&& p, R (C::*mem_func)(A1), fc::thread* t = nullptr) {
          return [=](A1 a1){ return t->async( [=](){ return (p->*mem_func)(a1); } ); };
        }
    };

    template<typename ThisPtr>
    struct actor_vtable_visitor {
        template<typename U>
        actor_vtable_visitor( fc::thread* t, U&& u ):_thread(t),_this( fc::forward<U>(u) ){}

        template<typename Function, typename MemberPtr>
        void operator()( const char* name, Function& memb, MemberPtr m )const {
          memb = actor_member::functor( _this, m, _thread );
        }
        fc::thread* _thread;
        ThisPtr _this;
    };
  }

  /**
   *  Posts all method calls to another thread and
   *  returns a future.
   */
  template<typename Interface>
  class actor : public ptr<Interface, detail::actor_member> {
    public:
      template<typename InterfaceType>
      actor( InterfaceType* p, fc::thread* t = &fc::thread::current() )
      {
          this->_vtable.reset(new detail::vtable<Interface,detail::actor_member>() );
          this->_vtable->template visit<InterfaceType>( detail::actor_vtable_visitor<InterfaceType*>(t, p) );
      }
      
  };

} // namespace fc
