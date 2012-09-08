#ifndef _FC_UTILITY_HPP_
#define _FC_UTILITY_HPP_
#include <stdint.h>
#include <new>

typedef decltype(sizeof(int)) size_t;
namespace std {
  typedef decltype(sizeof(int)) size_t;
}

namespace fc {
  template<typename T> struct remove_reference           { typedef T type;       };
  template<typename T> struct remove_reference<T&>       { typedef T type;       };
  template<typename T> struct remove_reference<const T&> { typedef const T type; };
  template<typename T> struct remove_reference<T&&>      { typedef T type;       };

  template<typename T>
  typename fc::remove_reference<T>::type&& move( T&& t ) { return static_cast<typename fc::remove_reference<T>::type&&>(t); }

  template<typename T, typename U>
  inline T&& forward( U&& u ) { return static_cast<T&&>(u); }

  namespace detail {
    template<typename T> char is_class_helper(void(T::*)());
    template<typename T> double is_class_helper(...);
  }
  template<typename T>
  struct is_class {
    enum { value = sizeof(char) == sizeof(detail::is_class_helper<T>(0)) }; 
  };

  template<typename T>
  void swap( T& a, T& b ) {
    T tmp = fc::move(a);
    a = fc::move(b);
    b = fc::move(tmp);
  }
}
#endif // _FC_UTILITY_HPP_
