#include <fc/ssh/client.hpp>
#include <fc/exception.hpp>
#include <fc/log.hpp>
#include <fc/iostream.hpp>
#include <iostream>

int main( int argc, char** argv ) {
  try {
    if( argc < 3 ) {
      fc::cout<<argv[0]<<" local_path  remote_path\n";
      return -1;
    }
   fc::cout<<"password: ";
   std::string pw;
   std::cin>>pw;
   fc::ssh::client c;
   c.connect( "dlarimer", pw, "localhost" );
   c.scp_send( argv[1], argv[2] );
  } catch ( ... ) {
   wlog( "%s", fc::except_str().c_str() ); 
  }
  return 0;
}
