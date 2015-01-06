#include <fc/uint128.hpp>
#include <fc/variant.hpp>
#include <fc/crypto/bigint.hpp>
#include <stdexcept>
#include "byteswap.hpp"

namespace fc 
{
    template <typename T>
    static void divide(const T &numerator, const T &denominator, T &quotient, T &remainder) 
    {

      static const int bits = sizeof(T) * 8;//CHAR_BIT;

      if(denominator == 0) {
        throw std::domain_error("divide by zero");
      } else {
        T n      = numerator;
        T d      = denominator;
        T x      = 1;
        T answer = 0;


        while((n >= d) && (((d >> (bits - 1)) & 1) == 0)) {
          x <<= 1;
          d <<= 1;
        }

        while(x != 0) {
          if(n >= d) {
            n -= d;
            answer |= x;
          }

          x >>= 1;
          d >>= 1;
        }

        quotient = answer;
        remainder = n;
      }
    }

    uint128::uint128(const std::string &sz) 
    :hi(0), lo(0) 
    {
      // do we have at least one character?
      if(!sz.empty()) {
        // make some reasonable assumptions
        int radix = 10;
        bool minus = false;

        std::string::const_iterator i = sz.begin();

        // check for minus sign, i suppose technically this should only apply
        // to base 10, but who says that -0x1 should be invalid?
        if(*i == '-') {
          ++i;
          minus = true;
        }

        // check if there is radix changing prefix (0 or 0x)
        if(i != sz.end()) {
          if(*i == '0') {
            radix = 8;
            ++i;
            if(i != sz.end()) {
              if(*i == 'x') {
                radix = 16;
                ++i;
              }
            }
          }

          while(i != sz.end()) {
            unsigned int n = 0;
            const char ch = *i;

            if(ch >= 'A' && ch <= 'Z') {
              if(((ch - 'A') + 10) < radix) {
                n = (ch - 'A') + 10;
              } else {
                break;
              }
            } else if(ch >= 'a' && ch <= 'z') {
              if(((ch - 'a') + 10) < radix) {
                n = (ch - 'a') + 10;
              } else {
                break;
              }
            } else if(ch >= '0' && ch <= '9') {
              if((ch - '0') < radix) {
                n = (ch - '0');
              } else {
                break;
              }
            } else {
              /* completely invalid character */
              break;
            }

            (*this) *= radix;
            (*this) += n;

            ++i;
          }
        }

        // if this was a negative number, do that two's compliment madness :-P
        if(minus) {
          *this = -*this;
        }
      }
    }


    uint128::operator bigint()const
    {
       auto tmp  = uint128( bswap_64( hi ), bswap_64( lo ) );
       bigint bi( (char*)&tmp, sizeof(tmp) );
       return bi;
    }
    uint128::uint128( const fc::bigint& bi )
    {
       *this = uint128( std::string(bi) ); // TODO: optimize this...
    }

    uint128::operator std::string ()const
    {
      if(*this == 0) { return "0"; }

      // at worst it will be size digits (base 2) so make our buffer
      // that plus room for null terminator
      static char sz [128 + 1];
       sz[sizeof(sz) - 1] = '\0';

      uint128 ii(*this);
      int i = 128 - 1;

      while (ii != 0 && i) {

      uint128 remainder;
      divide(ii, uint128(10), ii, remainder);
          sz [--i] = "0123456789abcdefghijklmnopqrstuvwxyz"[remainder.to_integer()];
      }

      return &sz[i];
    }


    uint128& uint128::operator<<=(const uint128& rhs) 
    {
        unsigned int n = rhs.to_integer();
        
        if(n >= 128) 
        {
          hi = 0;
          lo = 0;
        } 
        else 
        {
          const unsigned int halfsize = 128 / 2;
        
            if(n >= halfsize){
                n -= halfsize;
                hi = lo;
                lo = 0;
            }
        
            if(n != 0) {
            // shift high half
                hi <<= n;
        
            const uint64_t mask(~(uint64_t(-1) >> n));
        
            // and add them to high half
                hi |= (lo & mask) >> (halfsize - n);
        
            // and finally shift also low half
                lo <<= n;
            }
       }

        return *this;
    }

    uint128 & uint128::operator>>=(const uint128& rhs) 
    {
       unsigned int n = rhs.to_integer();
       
       if(n >= 128) {
         hi = 0;
         lo = 0;
       } else {
         const unsigned int halfsize = 128 / 2;
       
           if(n >= halfsize) {
               n -= halfsize;
               lo = hi;
               hi = 0;
           }
       
           if(n != 0) {
           // shift low half
               lo >>= n;
       
           // get lower N bits of high half
           const uint64_t mask(~(uint64_t(-1) << n));
       
           // and add them to low qword
               lo |= (hi & mask) << (halfsize - n);
       
           // and finally shift also high half
               hi >>= n;
           }
      }
      return *this;
   }

    uint128& uint128::operator/=(const uint128 &b) 
    {
        uint128 remainder;
        divide(*this, b, *this, remainder);
        return *this;
    }

    uint128& uint128::operator%=(const uint128 &b) 
    {
        uint128 quotient;
        divide(*this, b, quotient, *this);
        return *this;
    }

    uint128& uint128::operator*=(const uint128 &b) 
    {
      // check for multiply by 0
      // result is always 0 :-P
      if(b == 0) {
        hi = 0;
        lo = 0;
      } else if(b != 1) {
      
        // check we aren't multiplying by 1
      
          uint128 a(*this);
          uint128 t = b;
      
          lo = 0;
          hi = 0;
      
          for (unsigned int i = 0; i < 128; ++i) {
              if((t & 1) != 0) {
                  *this += (a << i);
          }
      
              t >>= 1;
          }
      }
      
        return *this;
   }
   
   void uint128::full_product( const uint128& a, const uint128& b, uint128& result_hi, uint128& result_lo )
   {
       //   (ah * 2**64 + al) * (bh * 2**64 + bl)
       // = (ah * bh * 2**128 + al * bh * 2**64 + ah * bl * 2**64 + al * bl
       // =  P * 2**128 + (Q + R) * 2**64 + S
       // = Ph * 2**192 + Pl * 2**128
       // + Qh * 2**128 + Ql * 2**64
       // + Rh * 2**128 + Rl * 2**64
       // + Sh * 2**64  + Sl
       //
       
       uint64_t ah = a.hi;
       uint64_t al = a.lo;
       uint64_t bh = b.hi;
       uint64_t bl = b.lo;

       uint128 s = al;
       s *= bl;
       uint128 r = ah;
       r *= bl;
       uint128 q = al;
       q *= bh;
       uint128 p = ah;
       p *= bh;
       
       uint64_t sl = s.lo;
       uint64_t sh = s.hi;
       uint64_t rl = r.lo;
       uint64_t rh = r.hi;
       uint64_t ql = q.lo;
       uint64_t qh = q.hi;
       uint64_t pl = p.lo;
       uint64_t ph = p.hi;

       uint64_t y[4];    // final result
       y[0] = sl;
       
       uint128_t acc = sh;
       acc += ql;
       acc += rl;
       y[1] = acc.lo;
       acc = acc.hi;
       acc += qh;
       acc += rh;
       acc += pl;
       y[2] = acc.lo;
       y[3] = acc.hi + ph;
       
       result_hi = uint128( y[3], y[2] );
       result_lo = uint128( y[1], y[0] );
       
       return;
   }
   
   void to_variant( const uint128& var,  variant& vo )  { vo = std::string(var);         }
   void from_variant( const variant& var,  uint128& vo ){ vo = uint128(var.as_string()); }

} // namespace fc


/*
 * Portions of the above code were adapted from the work of Evan Teran.
 *
 * Copyright (c) 2008
 * Evan Teran
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appears in all copies and that both the
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the same name not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission. We make no representations about the
 * suitability this software for any purpose. It is provided "as is"
 * without express or implied warranty.
 */

