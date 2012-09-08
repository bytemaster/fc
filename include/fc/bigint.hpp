#ifndef _FC_BIGINT_HPP
#define _FC_BIGINT_HPP
#include <stdint.h>
#include <fc/string.hpp>

struct bignum_st;
typedef bignum_st BIGNUM;

namespace fc {
  class bigint {
    public:
      bigint( const char* bige, uint32_t l );
      bigint( unsigned long i = 0 );
      bigint( const bigint& c );
      bigint( bigint&& c );
      ~bigint();

      bool    is_negative()const;
      int64_t to_int64()const;

      int64_t log2()const;
      bool operator < ( const bigint& c )const;
      bool operator > ( const bigint& c )const;
      bool operator >= ( const bigint& c )const;
      bool operator == ( const bigint& c )const;

      bigint operator + ( const bigint& a )const;
      bigint operator * ( const bigint& a )const;
      bigint operator / ( const bigint& a )const; 
      bigint operator - ( const bigint& a )const;

      bigint& operator = ( const bigint& a );
      bigint& operator = ( bigint&& a );

      operator fc::string()const;

    private:
      BIGNUM* n;
  };
} // namespace fc

#endif
