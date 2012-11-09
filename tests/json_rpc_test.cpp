#include <fc/json_rpc_stream_connection.hpp>
#include <fc/json_rpc_client.hpp>
#include <fc/json_rpc_tcp_server.hpp>
#include <fc/json_rpc_tcp_connection.hpp>
#include <fc/ip.hpp>
#include <fc/iostream.hpp>

struct namep {
  fc::string a;
  double     b;
};

FC_REFLECT( namep, (a)(b) )
FC_JSON_NAMED_PARAMS( namep )

struct test {
  int add(int x){ return x+1; }
  int sub(int x){ return x-1; }
  int sub1(int x){ return 3; }
  int namep_test(namep x){ return 3; }
  int sub2(float x){ return 3; }
  int sub3(double x, int y){ return x-y; }
  int sub4(uint16_t x){ return 3; }
  int sub5(char x){ return 3; }
  int sub6(uint64_t x){ return 3; }
  int sub7(int x){ return 3; }
  int sub8(int x){ return 3; }
  int sub9(int x){ return 3; }
};

FC_STUB( test, (add)(namep_test)(sub)(sub1)(sub2)(sub3)(sub4)(sub5)(sub6)(sub7)(sub8)(sub9) )

int main( int argc, char** argv ) {
  try {
  fc::ptr<test> t( new test() );
  fc::json::rpc_tcp_server serv;
  serv.add_interface( t );
  serv.listen(8001);
  slog( "%s", fc::json::to_string( fc::json::detail::named_param<fc::tuple<namep> >::to_value(fc::tuple<namep>() ) ).c_str() );
  slog("...");
  {
      wlog( "create new connection" );
      fc::json::rpc_tcp_connection::ptr con(new fc::json::rpc_tcp_connection());
      wlog( "connnect to..." );
      con->connect_to( fc::ip::endpoint::from_string("127.0.0.1:8001") );
      wlog( "connected, " );

      fc::json::rpc_client<test> rpcc( con );
      slog( "5+1=%d", rpcc->add(5).wait() );
      slog( "sub3 4-5=%d", rpcc->sub3(4,5).wait() );
      slog( "namep=%d", rpcc->namep_test(namep()).wait() );
  }
  slog( "exit serv" );
  /*
  fc::json::rpc_connection::ptr con( new fc::json::rpc_stream_connection( fc::cin, fc::cout ) );
  fc::json::rpc_client<test> c( con );

  slog( "%d", c->add( 5 ).wait() );
  slog( "%d", c->add( 6 ).wait() );
  */

  slog( "Exiting" );
  } catch ( ... ) {
    elog( "%s", fc::except_str().c_str() );
  }

  return 0;
}
