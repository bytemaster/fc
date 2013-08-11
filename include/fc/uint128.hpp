#pragma once
#include <stdint.h>
#include <string>

namespace fc 
{
  class bigint;
  /**
   *  @brief an implementation of 128 bit unsigned integer
   *
   */
  class uint128 
  {


     public:
      uint128():hi(0),lo(0){};
      uint128( uint32_t l ):hi(0),lo(l){}
      uint128( int32_t l ):hi( -(l<0) ),lo(l){}
      uint128( int64_t l ):hi( -(l<0) ),lo(l){}
      uint128( uint64_t l ):hi(0),lo(l){}
      uint128( const std::string& s );
      uint128( uint64_t _h, uint64_t _l )
      :hi(_h),lo(_l){}
      uint128( const fc::bigint& bi );

      operator std::string()const;
      operator fc::bigint()const;

      bool     operator == ( const uint128& o )const{ return hi == o.hi && lo == o.lo;             }
      bool     operator != ( const uint128& o )const{ return hi != o.hi || lo != o.lo;             }
      bool     operator < ( const uint128& o )const { return (hi == o.hi) ? lo < o.lo : hi < o.hi; }
      bool     operator !()const                    { return !(hi !=0 || lo != 0);                 }
      uint128  operator -()const                    { return ++uint128( ~hi, ~lo );                }
      uint128  operator ~()const                    { return uint128( ~hi, ~lo );                  }

      uint128& operator++()    {  hi += (++lo == 0); return *this; }
      uint128& operator--()    {  hi -= (lo-- == 0); return *this; }
      uint128  operator++(int) { auto tmp = *this; ++(*this); return tmp; } 
      uint128  operator--(int) { auto tmp = *this; --(*this); return tmp; }

      uint128& operator |= ( const uint128& u ) { hi |= u.hi; lo |= u.lo; return *this; }
      uint128& operator &= ( const uint128& u ) { hi &= u.hi; lo &= u.lo; return *this; }
      uint128& operator ^= ( const uint128& u ) { hi ^= u.hi; lo ^= u.lo; return *this; }
      uint128& operator <<= ( const uint128& u );
      uint128& operator >>= ( const uint128& u );

      uint128& operator += ( const uint128& u ) { const uint64_t old = lo; lo += u.lo;  hi += u.hi + (lo < old); return *this; }
      uint128& operator -= ( const uint128& u ) { return *this += -u; }
      uint128& operator *= ( const uint128& u );
      uint128& operator /= ( const uint128& u );
      uint128& operator %= ( const uint128& u );


      friend uint128 operator + ( const uint128& l, const uint128& r )   { return uint128(l)+=r;   }
      friend uint128 operator - ( const uint128& l, const uint128& r )   { return uint128(l)-=r;   }
      friend uint128 operator * ( const uint128& l, const uint128& r )   { return uint128(l)*=r;   }
      friend uint128 operator / ( const uint128& l, const uint128& r )   { return uint128(l)/=r;   }
      friend uint128 operator | ( const uint128& l, const uint128& r )   { return uint128(l)=(r);  }
      friend uint128 operator & ( const uint128& l, const uint128& r )   { return uint128(l)&=r;   }
      friend uint128 operator ^ ( const uint128& l, const uint128& r )   { return uint128(l)^=r;   }
      friend uint128 operator << ( const uint128& l, const uint128& r )  { return uint128(l)<<=r;  }
      friend uint128 operator >> ( const uint128& l, const uint128& r )  { return uint128(l)>>=r;  }
      friend bool    operator >  ( const uint128& l, const uint128& r )  { return r < l;           }


      friend bool    operator >=  ( const uint128& l, const uint128& r ) { return l == r || l > r; }
      friend bool    operator <=  ( const uint128& l, const uint128& r ) { return l == r || l < r; }

      uint32_t to_integer()const { return (uint32_t)lo; }
      uint64_t to_uint64()const { return lo; }
      uint64_t low_bits()const  { return lo; }
      uint64_t high_bits()const { return hi; }

      private:
          uint64_t hi;
          uint64_t lo;
      
  };
  static_assert( sizeof(uint128) == 2*sizeof(uint64_t), "validate packing assumptions" );

  typedef uint128 uint128_t;

  class variant;

  void to_variant( const uint128& var,  variant& vo );
  void from_variant( const variant& var,  uint128& vo );

  namespace raw  
  {
    template<typename Stream>
    inline void pack( Stream& s, const uint128& u ) { s.write( (char*)&u, sizeof(u) ); }
    template<typename Stream>
    inline void unpack( Stream& s, uint128& u ) { s.read( (char*)&u, sizeof(u) ); }
  }

} // namespace fc
