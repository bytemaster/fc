#ifndef _FC_VECTOR_FWD_HPP_
#define _FC_VECTOR_FWD_HPP_
namespace fc {
  template<typename T> class vector;
  template<typename T> class reflector;
  template<typename T> class reflector< fc::vector<T> >;
};
#endif // _FC_VECTOR_FWD_HPP_
