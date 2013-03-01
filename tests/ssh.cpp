#include <fc/ssh/client.hpp>
#include <fc/exception.hpp>
#include <fc/log.hpp>
#include <fc/iostream.hpp>
//#include <iostream>

int main( int argc, char** argv ) {
  try {
   slog( "create ssh client" );
   fc::ssh::client c;
   c.connect( "dlarimer", "rapture", "10.10.10.112" );
   slog( "connected" );
   fc::ssh::process proc = c.exec( "/bin/cat -u" );
   slog( "proc!");
   fc::string hello( "hello.............." );
   hello += hello;
   hello += hello;
   hello += hello;
   hello += hello;
   hello += hello;
   hello += "\n";
   /*
   hello += hello2;
   */
   fc::string line;
   proc.in_stream().write(hello.c_str(), hello.size() );
   fc::getline( proc.out_stream(), line );
   fc::cout<<line<<"\n";
   while( !proc.out_stream().eof() ) {
      fc::cout<<line<<"\n";
      proc.in_stream().write(hello.c_str(), hello.size() );
      proc.in_stream().flush();
      fc::getline( proc.out_stream(), line );
   }
   fc::cout<<"result: "<<proc.result()<<"\n";
   /*
   while( true ) {
      c.scp_send( argv[1], argv[2] );
   }
   */
  } catch ( ... ) {
   wlog( "%s", fc::except_str().c_str() ); 
  }
  return 0;
}
