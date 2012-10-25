#ifndef _FC_FUNCTION_HPP_
#define _FC_FUNCTION_HPP_
#include <fc/utility.hpp>
#include <fc/shared_ptr.hpp>
/*
#include <functional>
#include <boost/config.hpp>

namespace fc {
  // place holder for more compile-effecient functor
#if !defined(BOOST_NO_TEMPLATE_ALIASES) 
  template<typename T>
  using function = std::function<T>;
#else
#endif
*/


//template<typename Signature>
//class function { };

namespace fc {

template<typename R,typename ... Args>
class function_impl {
  public:
    template<typename Functor>
    function_impl( Functor&& f )
    :func( new impl<Functor>( fc::forward<Functor>(f) ) ){};

    function_impl( const function_impl& c ):func(c.func){}
    function_impl( function_impl&& c ) { fc::swap( func, c.func); }
    ~function_impl(){}

    template<typename Functor>
    function_impl& operator=( Functor&& f ) { 
      func.reset( new impl<Functor>( fc::forward<Functor>(f) ) ); 
      return *this; 
    }

    function_impl& operator=( const function_impl& c ) { func = c.func; return *this; }
    function_impl& operator=( function_impl&& c )      { fc::swap(func,c.func); return *this; }
    
    template<typename... Args2>
    R operator()( Args2... args2) { return func->call(fc::forward<Args2>(args2)...); }

  private:
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
 */
template<typename Signature = void()>
class function : public function_impl<void> {
  public:
    template<typename U>
    function( U&& u ):function_impl<void>( fc::forward<U>(u) ){}
    using function_impl::operator=;
};

template<typename R>
class function<R()> : public function_impl<R> {
  public:
    template<typename U>
    function( U&& u ):function_impl<R>( fc::forward<U>(u) ){}
    using function_impl<R>::operator=;
};

template<typename R,typename A1>
class function<R(A1)> : public function_impl<R,A1> {
  public:
    template<typename U>
    function( U&& u ):function_impl<R,A1>( fc::forward<U>(u) ){}
    using function_impl<R,A1>::operator=;
};

template<typename R,typename A1,typename A2>
class function<R(A1,A2)> : public function_impl<R,A1,A2> {
  public:
    template<typename U>
    function( U&& u ):function_impl<R,A1,A2>( fc::forward<U>(u) ){}
    using function_impl<R,A1,A2>::operator=;
};






}

#endif // _FC_FUNCTION_HPP_
