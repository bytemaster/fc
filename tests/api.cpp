#include <fc/api.hpp>
#include <fc/log/logger.hpp>
//#include <fc/rpc/api_server.hpp>

class calculator
{
   public:
      int32_t add( int32_t a, int32_t b ); // not implemented
      int32_t sub( int32_t a, int32_t b ); // not implemented
};

FC_API( calculator, (add)(sub) )

using namespace fc;

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

template<typename R, typename Arg0, typename ... Args>
//std::function<R(Args...)> bind_first_arg( const std::function<R(Arg0,Args...)>& f, Arg0 ao )
std::function<R(Args...)> bind_first_arg( const std::function<R(Arg0,Args...)>& f, Arg0 ao )
{
   return [=]( Args... args ) { return f( ao, args... ); };
}

template<typename R, typename Arg0>
R call_generic( const std::function<R(Arg0)>& f, variants::const_iterator a0, variants::const_iterator e )
{
   return f(a0->as<Arg0>());
}
template<typename R, typename Arg0, typename ... Args>
R call_generic( const std::function<R(Arg0,Args...)>& f, variants::const_iterator a0, variants::const_iterator e )
{
   return  call_generic<R,Args...>( bind_first_arg( f, a0->as<Arg0>() ), a0+1, e );
}

template<typename R, typename ... Args>
std::function<variant(const fc::variants&)> to_generic( const std::function<R(Args...)>& f )
{
   return [=]( const variants& args ) { return call_generic( f, args.begin(), args.end() ); };
}

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

   variants v = { 4, 5 };
   auto g = to_generic( api_calc->add );
   auto r = call_generic( api_calc->add, v.begin(), v.end() );
   wdump((r));
   wdump( (g(v)) );


 //  fc::api_server server;
 //  server.register_api( api_calc );
   return 0;
}
