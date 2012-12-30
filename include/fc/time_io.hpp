#pragma once
#include <fc/time_point.hpp>
#include <fc/raw.hpp>

namespace fc {
  namespace raw {
    template<typename Stream, typename T>
    void unpack( Stream& s, fc::time_point& v ) {
      int64_t micro;  
      fc::raw::unpack(s, micro );
      v = fc::time_point( fc::microseconds(micro);
    }
    template<typename Stream, typename T>
    void pack( Stream& s, const fc::time_point& v ) {
      fc::raw::pack( s, v.time_since_epoch().count() );
    }
  }
}
