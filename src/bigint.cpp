#include <fc/bigint.hpp>
#include <fc/utility.hpp>
#include <openssl/bn.h>

namespace fc {
      bigint::bigint( const char* bige, uint32_t l ) {
        n = BN_bin2bn( (const unsigned char*)bige, l, NULL );
      }

      bigint::bigint( unsigned long i )
      :n(BN_new()) {
        BN_set_word( n, i );
      }

      bigint::bigint( const bigint& c ) {
        n = BN_dup( c.n );
      }

      bigint::bigint( bigint&& b ) {
        n = b.n;
        b.n = 0;
      }

      bigint::~bigint() {
        if(n!=0) BN_free(n);
      }

      bool bigint::is_negative()const { return BN_is_negative(n); }
      int64_t bigint::to_int64()const { return BN_get_word(n); }

      int64_t bigint::log2()const { return BN_num_bits(n); }
      bool bigint::operator < ( const bigint& c )const {
        return BN_cmp( n, c.n ) < 0;
      }
      bool bigint::operator > ( const bigint& c )const {
        return BN_cmp( n, c.n ) > 0;
      }
      bool bigint::operator >= ( const bigint& c )const {
        return BN_cmp( n, c.n ) >= 0;
      }
      bool bigint::operator == ( const bigint& c )const {
        return BN_cmp( n, c.n ) == 0;
      }

      bigint bigint::operator + ( const bigint& a )const {
        bigint tmp(*this);
        BN_add( tmp.n, n, a.n );
        return tmp;
      }
      bigint bigint::operator * ( const bigint& a )const {
        BN_CTX* ctx = BN_CTX_new();
        bigint tmp(*this);
        BN_mul( tmp.n, n, a.n, ctx );
        BN_CTX_free(ctx);
        return tmp;
      }
      bigint bigint::operator / ( const bigint& a ) const {
        BN_CTX* ctx = BN_CTX_new();
        bigint tmp(*this);
        BN_div( tmp.n, NULL, n, a.n, ctx );
        BN_CTX_free(ctx);
        return tmp;
      }
      bigint bigint::operator - ( const bigint& a )const {
        bigint tmp(*this);
        BN_sub( tmp.n, n, a.n );
        return tmp;
      }


      bigint& bigint::operator = ( bigint&& a ) {
        fc::swap( a.n, n );
        return *this;
      }
      bigint& bigint::operator = ( const bigint& a ) {
        if( &a == this ) 
          return *this;
        BN_copy( n, a.n );
        return *this;
      }
      bigint::operator fc::string()const {
        return BN_bn2dec(n);
      }
} // namespace fc 
