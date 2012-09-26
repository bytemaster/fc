#ifndef _FC_ARRAY_HPP_
#define _FC_ARRAY_HPP_

namespace fc {

  template<typename T, size_t N>
  class array {
    public:
    T data[N];
  };

}

#endif // _FC_ARRAY_HPP_
