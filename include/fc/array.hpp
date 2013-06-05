#pragma once
#include <fc/crypto/base64.hpp>
#include <fc/variant.hpp>

namespace fc {

  template<typename T, size_t N>
  class array {
    public:
    T data[N];
  };

  template<typename T, size_t N>
  bool operator == ( const array<T,N>& a, const array<T,N>& b )
  { return 0 == memcmp( a.data, b.data, N ); }
  template<typename T, size_t N>
  bool operator != ( const array<T,N>& a, const array<T,N>& b )
  { return 0 != memcmp( a.data, b.data, N ); }

  template<typename T, size_t N>
  void to_variant( const array<T,N>& bi, variant& v )
  {
     v = fc::vector<char>( (const char*)&bi, ((const char*)&bi) + sizeof(bi) );
  }
  template<typename T, size_t N>
  void from_variant( const variant& v, array<T,N>& bi )
  {
    fc::vector<char> ve = v.as< vector<char> >();
    if( ve.size() )
    {
        memcpy(&bi, ve.data(), fc::min<size_t>(ve.size(),sizeof(bi)) );
    }
    else
        memset( &bi, char(0), sizeof(bi) );
  }
}
