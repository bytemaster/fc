#include <fc/network/udt_socket.hpp>
#include <fc/network/ip.hpp>
#include <fc/exception/exception.hpp>
#include <iostream>
#include <vector>

using namespace fc;

int main( int argc, char** argv )
{
   try {
       udt_socket sock;
       sock.bind( fc::ip::endpoint::from_string( "127.0.0.1:6666" ) );
       sock.connect_to( fc::ip::endpoint::from_string( "127.0.0.1:7777" ) );

       std::cout << "local endpoint: " <<std::string( sock.local_endpoint() ) <<"\n";
       std::cout << "remote endpoint: " <<std::string( sock.remote_endpoint() ) <<"\n";

       std::string hello = "hello world";
       sock.write( hello.c_str(), hello.size() );

       std::vector<char> response;
       response.resize(1024);
       int r = sock.readsome( response.data(), response.size() );

       std::cout << "response: '"<<std::string( response.data(), r ) <<"'\n";
   } catch ( const fc::exception& e )
   {
      elog( "${e}", ("e",e.to_detail_string() ) );
   }

    return 0;
}
