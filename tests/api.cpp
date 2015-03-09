#include <fc/api.hpp>
#include <fc/log/logger.hpp>
#include <fc/rpc/api_server.hpp>

class calculator
{
   public:
      int32_t add( int32_t a, int32_t b ); // not implemented
      int32_t sub( int32_t a, int32_t b ); // not implemented
};

FC_API( calculator, (add)(sub) )

class some_calculator
{
   public:
      int32_t add( int32_t a, int32_t b ) { return a+b; }
      int32_t sub( int32_t a, int32_t b ) { return a-b; }
};
class variant_calculator
{
   public:
      double add( fc::variant a, fc::variant b ) { return a.as_double()+b.as_double(); }
      double sub( fc::variant a, fc::variant b ) { return a.as_double()-b.as_double(); }
};


int main( int argc, char** argv )
{
   some_calculator calc;
   variant_calculator vcalc;
   fc::api<calculator> api_calc( &calc );
   fc::api<calculator> api_vcalc( &vcalc );
   fc::api<calculator> api_nested_calc( api_calc );
   wdump( (api_calc->add(5,4)) );
   wdump( (api_calc->sub(5,4)) );
   wdump( (api_vcalc->add(5,4)) );
   wdump( (api_vcalc->sub(5,4)) );
   wdump( (api_nested_calc->sub(5,4)) );
   wdump( (api_nested_calc->sub(5,4)) );

   fc::api_server server;
   server.register_api( api_calc );
   return 0;
}
