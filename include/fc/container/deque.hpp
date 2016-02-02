#pragma once

#include <deque>
#include <fc/io/raw.hpp>

namespace fc {
   namespace raw {

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::deque<T>& value ) {
      pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fc::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::deque<T>& value ) {
      unsigned_int size; unpack( s, size );
      FC_ASSERT( size.value*sizeof(T) < MAX_ARRAY_ALLOC_SIZE );
      value.resize(size.value);
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fc::raw::unpack( s, *itr );
        ++itr;
      }
    }

    } // namespace raw

} // namespace fc
