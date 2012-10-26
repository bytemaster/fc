#ifndef _FC_VECTOR_FWD_HPP_
#define _FC_VECTOR_FWD_HPP_
#if 0
#include <fc/vector.hpp>
#else
namespace fc {
  template<typename T> class vector;
  template<typename T> struct reflector;
  template<typename T> struct reflector< fc::vector<T> >;
};
#endif

#endif // _FC_VECTOR_FWD_HPP_
