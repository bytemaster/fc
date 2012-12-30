#pragma once
#include <fc/string.hpp>

namespace fc {
  uint32_t super_fast_hash (const char * data, int len);
  uint32_t super_fast_hash (const fc::string& str );
}


