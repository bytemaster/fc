#ifndef _FC_FUNCTION_HPP_
#define _FC_FUNCTION_HPP_
#include <fc/utility.hpp>
#include <functional>

namespace fc {
  // place holder for more compile-effecient functor
  template<typename T>
  using function = std::function<T>;
}

#endif // _FC_FUNCTION_HPP_
