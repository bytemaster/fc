#ifndef _EXAMPLE_HPP_
#define _EXAMPLE_HPP_
#include <fc/reflect_fwd.hpp>

  struct example {
      int a;
      int b;
  };

  FC_REFLECTABLE( example )

#endif //  _EXAMPLE_HPP_
