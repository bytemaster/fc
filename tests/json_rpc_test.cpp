#include <fc/json_rpc_stream_connection.hpp>
#include <fc/json_rpc_client.hpp>
#include <fc/iostream.hpp>

struct test {
  int add(int x){ return x+1; }
  int sub(int x){ return x-1; }
  int sub1(int x){ return 3; }
  int sub2(float x){ return 3; }
  int sub3(double x){ return 3; }
  int sub4(uint16_t x){ return 3; }
  int sub5(char x){ return 3; }
  int sub6(uint64_t x){ return 3; }
  int sub7(int x){ return 3; }
  int sub8(int x){ return 3; }
  int sub9(int x){ return 3; }
};

FC_STUB( test, (add)(sub)(sub1)(sub2)(sub3)(sub4)(sub5)(sub6)(sub7)(sub8)(sub9) )

int main( int argc, char** argv ) {
  try {
  fc::json::rpc_connection::ptr con( new fc::json::rpc_stream_connection( fc::cin, fc::cout ) );
  fc::json::rpc_client<test> c( con );

  slog( "%d", c->add( 5 ).wait() );
  slog( "%d", c->add( 6 ).wait() );

  slog( "Exiting" );
  } catch ( ... ) {
    elog( "%s", fc::except_str().c_str() );
  }

  return 0;
}
