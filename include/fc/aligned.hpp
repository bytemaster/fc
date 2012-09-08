#ifndef _FC_ALIGNED_HPP_
#define _FC_ALIGNED_HPP_
namespace fc {

  template<unsigned int S, typename T=double>
  struct aligned {
    union {
      T    _align;
      char _data[S];
    } _store;
  };

}
#endif // _FC_ALIGNED_HPP_
