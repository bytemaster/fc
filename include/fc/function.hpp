#ifndef _FC_FUNCTION_HPP_
#define _FC_FUNCTION_HPP_
#include <fc/utility.hpp>
#include <functional>
#include <boost/config.hpp>

namespace fc {
  // place holder for more compile-effecient functor
#if !defined(BOOST_NO_TEMPLATE_ALIASES) 
  template<typename T>
  using function = std::function<T>;
#else
#endif
}

#endif // _FC_FUNCTION_HPP_
