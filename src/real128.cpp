#include <fc/real128.hpp>
#include <fc/crypto/bigint.hpp>
#include <fc/exception/exception.hpp>
#include <sstream>

namespace fc
{
   real128& real128::operator += ( const real128& o )
   {
      fixed += o.fixed;
      return *this;
   }
   real128& real128::operator -= ( const real128& o )
   {
      fixed -= o.fixed;
      return *this;
   }

   real128& real128::operator /= ( const real128& o )
   { try {
      FC_ASSERT( o.fixed > uint128(0), "Divide by Zero" );
       
      fc::bigint self(fixed);
      fc::bigint other(o.fixed);
      self *= fc::bigint(uint128(0,-1));
      self /= other;
      fixed = self;

      return *this;
   } FC_CAPTURE_AND_RETHROW( (*this)(o) ) }

   real128& real128::operator *= ( const real128& o )
   { try {
      fc::bigint self(fixed);
      fc::bigint other(o.fixed);
      self *= other;
      self /= fc::bigint(uint128(0,-1));
      fixed = self;
      return *this;
   } FC_CAPTURE_AND_RETHROW( (*this)(o) ) }


   real128::real128( const std::string& ratio_str )
   {
     const char* c = ratio_str.c_str();
     int digit = *c - '0';
     if (digit >= 0 && digit <= 9)
     {
       uint64_t int_part = digit;
       ++c;
       digit = *c - '0';
       while (digit >= 0 && digit <= 9)
       {
         int_part = int_part * 10 + digit;
         ++c;
         digit = *c - '0';
       }
       fixed = fc::uint128(int_part);
     }
     else
     {
       // if the string doesn't look like "123.45" or ".45", this code isn't designed to parse it correctly
       // in particular, we don't try to handle leading whitespace or '+'/'-' indicators at the beginning
       assert(*c == '.');
       fixed = fc::uint128();
     }


     if (*c == '.')
     {
       c++;
       digit = *c - '0';
       if (digit >= 0 && digit <= 9)
       {
         int64_t frac_part = digit;
         int64_t frac_magnitude = 10;
         ++c;
         digit = *c - '0';
         while (digit >= 0 && digit <= 9)
         {
           frac_part = frac_part * 10 + digit;
           frac_magnitude *= 10;
           ++c;
           digit = *c - '0';
         }
         *this += real128( frac_part ) / real128( frac_magnitude );
       }
     }
   }
   real128::operator std::string()const
   {
      std::stringstream ss;
      ss << to_uint64();
      ss << '.';
      auto frac = *this * real128( uint128(-1,0) );
      frac += real128(1);

      ss << std::string( frac.fixed ).substr(1);

      auto number = ss.str();
      while(  number.back() == '0' ) number.pop_back();

      return number;
   }

   void to_variant( const real128& var,  variant& vo )
   {
      vo = std::string(var);
   }
   void from_variant( const variant& var,  real128& vo )
   {
     vo = real128(var.as_string());
   }

} // namespace fc
