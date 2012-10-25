#pragma once
#include <fc/utility.hpp>
#include <fc/shared_ptr.hpp>

namespace fc {
template<typename R,typename ... Args>
class function {
  public:
    function(){}

    template<typename Functor>
    function( Functor&& f )
    :func( new impl<Functor>( fc::forward<Functor>(f) ) ){};

    function( const function& c ):func(c.func){}
    function( function&& c ) { fc::swap( func, c.func); }
    ~function(){}

    template<typename Functor>
    function& operator=( Functor&& f ) { 
      func.reset( new impl<Functor>( fc::forward<Functor>(f) ) ); 
      return *this; 
    }

    function& operator=( const function& c ) { func = c.func; return *this; }
    function& operator=( function&& c )      { fc::swap(func,c.func); return *this; }
    
    template<typename... Args2>
    R operator()( Args2... args2) { return func->call(fc::forward<Args2>(args2)...); }

  protected:

    struct impl_base : public fc::retainable {
      virtual ~impl_base(){}
      virtual R call(Args...) = 0;
    };

    template<typename Functor>
    struct impl  : impl_base {
      template<typename U>
      impl( U&& u ):func( fc::forward<U>(u) ){}

      virtual R call(Args... args) { return func(args...); }

      Functor func;
    };
    function( const fc::shared_ptr<impl_base>& f ):func(f){}
    function( fc::shared_ptr<impl_base>&& f ):func(fc::move(f)){}

    fc::shared_ptr<impl_base> func;
};

/**
 *  Provides functionality similar to boost::function.
 *
 *  Functions have 'reference semantics', meaning that copies will all
 *  refer to the same underlying function.
 *
 *  TODO: Small functions are allocated on the stack, large functors are
 *  allocated on the heap.
 *
 *  Simply including boost/function adds an additional 0.6 seconds to every 
 *  object file compared to using fc/function.
 *
 *  Including <functional> on the other hand adds a mere 0.05 
 *  seconds to every object file compared to fc/function.
 */
template<typename R>
class function<R()> : public function<R> {
  public:
    function(){}
    template<typename U>
    function( U&& u ) { *this = fc::forward<U>(u); }
    using function<R>::operator=;
};

template<typename R,typename A1>
class function<R(A1)> : public function<R,A1> {
  public:
    function(){}
    template<typename U>
    function( U&& u ) { *this = fc::forward<U>(u); }
    //using function<R,A1>::operator=;
};

template<typename R,typename A1,typename A2>
class function<R(A1,A2)> : public function<R,A1,A2> {
  public:
    function(){}
    template<typename U>
    function( U&& u ):function<R,A1,A2>( fc::forward<U>(u) ){}
    function( const function& c ):function<R,A1,A2>(c.func){}
    using function<R,A1,A2>::operator=;
};

template<typename R,typename A1,typename A2, typename A3>
class function<R(A1,A2,A3)> : public function<R,A1,A2,A3> {
  public:
    function(){}
    template<typename U>
    function( U&& u ):function<R,A1,A2,A3>( fc::forward<U>(u) ){}
    function( const function& c ):function<R,A1,A2,A3>(c.func){}
    using function<R,A1,A2,A3>::operator=;
};

}

