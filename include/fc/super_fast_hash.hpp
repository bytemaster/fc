#ifndef _FC_SUPER_FAST_HASH_HPP_
#define _FC_SUPER_FAST_HASH_HPP_

namespace fc {
  class string;
  uint32_t super_fast_hash (const char * data, int len);
  uint32_t super_fast_hash (const fc::string& str );
}


#endif // _FC_SUPER_FAST_HASH_HPP_
