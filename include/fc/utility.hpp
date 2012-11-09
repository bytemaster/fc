#ifndef _FC_UTILITY_HPP_
#define _FC_UTILITY_HPP_
#include <stdint.h>
#include <new>

#define nullptr 0

typedef decltype(sizeof(int)) size_t;
namespace std {
  typedef decltype(sizeof(int)) size_t;
}

namespace fc {
  template<typename T> struct remove_reference           { typedef T type;       };
  template<typename T> struct remove_reference<T&>       { typedef T type;       };
  template<typename T> struct remove_reference<T&&>      { typedef T type;       };

  template<typename T> struct deduce           { typedef T type; };
  template<typename T> struct deduce<T&>       { typedef T type; };
  template<typename T> struct deduce<const T&> { typedef T type; };
  template<typename T> struct deduce<T&&>      { typedef T type; };
  template<typename T> struct deduce<const T&&>{ typedef T type; };

  template<typename T>
  typename fc::remove_reference<T>::type&& move( T&& t ) { return static_cast<typename fc::remove_reference<T>::type&&>(t); }

  template<typename T, typename U>
  inline T&& forward( U&& u ) { return static_cast<T&&>(u); }

  struct true_type  { enum _value { value = 1 }; };
  struct false_type { enum _value { value = 0 }; };

  namespace detail {
    template<typename T> fc::true_type is_class_helper(void(T::*)());
    template<typename T> fc::false_type is_class_helper(...);
  }

  template<typename T>
  struct is_class { typedef decltype(detail::is_class_helper<T>(0)) type; enum value_enum { value = type::value }; };

  template<typename T>
  const T& min( const T& a, const T& b ) { return a < b ? a: b; }

  template<typename T>
  void swap( T& a, T& b ) {
    T tmp = fc::move(a);
    a = fc::move(b);
    b = fc::move(tmp);
  }
}
#endif // _FC_UTILITY_HPP_
