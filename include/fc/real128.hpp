#include <fc/uint128.hpp>   

namespace fc {
   class variant;

   /**
    * Provides fixed point math operations based on decimal fractions
    * with 18 places.
    * Delegates to fc::bigint for multiplication and division.
    */
   class real128
   {
      public:
         real128( uint64_t integer = 0);
         explicit real128( const std::string& str );
         operator std::string()const;

         friend real128 operator * ( real128 a, const real128& b ) { a *= b; return a; }
         friend real128 operator / ( real128 a, const real128& b ) { a /= b; return a; }
         friend real128 operator + ( real128 a, const real128& b ) { a += b; return a; }
         friend real128 operator - ( real128 a, const real128& b ) { a -= b; return a; }

         real128& operator += ( const real128& o );
         real128& operator -= ( const real128& o );
         real128& operator /= ( const real128& o );
         real128& operator *= ( const real128& o );

         uint64_t to_uint64()const;

      private:
         uint128  fixed;
   };

   void to_variant( const real128& var,  variant& vo );
   void from_variant( const variant& var,  real128& vo );

} // namespace fc
