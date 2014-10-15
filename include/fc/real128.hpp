#include <fc/uint128.hpp>   

namespace fc {
   class variant;

   /**
    * Provides 64.64 fixed point math operations
    * based upon base 2^64-1  
    */
   class real128
   {
      public:
         real128( uint64_t integer = 0):fixed(integer){}
         real128( const std::string& str );
         operator std::string()const;

         friend real128 operator * ( real128 a, const real128& b ) { a *= b; return a; }
         friend real128 operator / ( real128 a, const real128& b ) { a /= b; return a; }
         friend real128 operator + ( real128 a, const real128& b ) { a += b; return a; }
         friend real128 operator - ( real128 a, const real128& b ) { a -= b; return a; }

         real128& operator += ( const real128& o );
         real128& operator -= ( const real128& o );
         real128& operator /= ( const real128& o );
         real128& operator *= ( const real128& o );

         uint64_t to_uint64()const{ return fixed.high_bits(); }

      private:
         uint128  fixed;
   };

   void to_variant( const real128& var,  variant& vo );
   void from_variant( const variant& var,  real128& vo );

} // namespace fc
