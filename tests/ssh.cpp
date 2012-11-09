#include <fc/ssh/client.hpp>
#include <fc/exception.hpp>
#include <fc/log.hpp>
#include <fc/iostream.hpp>
//#include <iostream>

int main( int argc, char** argv ) {
  try {
 //   if( argc < 3 ) {
 //     fc::cout<<argv[0]<<" local_path  remote_path\n";
  //    return -1;
  //  }
   fc::cout<<"password: ";
//   fc::string pw;
//   std::cin>>pw;
   fc::ssh::client c;
   c.connect( "dlarimer", "", "localhost" );
   fc::ssh::process proc = c.exec( "/bin/ls" );
   while( !proc.out_stream().eof() ) {
      fc::string line;
      fc::getline( proc.out_stream(), line );
      fc::cout<<line<<"\n";
   }
   fc::cout<<"result: "<<proc.result()<<"\n";
//   c.scp_send( argv[1], argv[2] );
  } catch ( ... ) {
   wlog( "%s", fc::except_str().c_str() ); 
  }
  return 0;
}
