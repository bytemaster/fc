#pragma once
#include <fc/crypto/base64.hpp>
#include <fc/variant.hpp>

namespace fc {

  /**
   *  Provides a fixed size array that is easier for templates to specialize 
   *  against or overload than T[N].  
   */
  template<typename T, size_t N>
  class array {
    public:
    /**
     *  Checked indexing (when in debug build) that also simplifies dereferencing
     *  when you have an array<T,N>*.    
     */
    ///@{
    T&       at( size_t pos )      { assert( pos < N); return data[pos]; }
    const T& at( size_t pos )const { assert( pos < N); return data[pos]; }
    ///@}
    
    T*           begin()       {  return &data[0]; }
    const T*     begin()const  {  return &data[0]; }
    const T*     end()const    {  return &data[N]; }

    size_t       size()const { return N; }
    
    T data[N];
  };

  template<typename T, size_t N>
  bool operator == ( const array<T,N>& a, const array<T,N>& b )
  { return 0 == memcmp( a.data, b.data, N*sizeof(T) ); }

  template<typename T, size_t N>
  bool operator != ( const array<T,N>& a, const array<T,N>& b )
  { return 0 != memcmp( a.data, b.data, N*sizeof(T) ); }

  template<typename T, size_t N>
  void to_variant( const array<T,N>& bi, variant& v )
  {
     v = std::vector<char>( (const char*)&bi, ((const char*)&bi) + sizeof(bi) );
  }
  template<typename T, size_t N>
  void from_variant( const variant& v, array<T,N>& bi )
  {
    std::vector<char> ve = v.as< std::vector<char> >();
    if( ve.size() )
    {
        memcpy(&bi, ve.data(), fc::min<size_t>(ve.size(),sizeof(bi)) );
    }
    else
        memset( &bi, char(0), sizeof(bi) );
  }
}
