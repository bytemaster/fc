#include <fc/http/connection.hpp>
#include <fc/tcp_socket.hpp>
#include <fc/sstream.hpp>
#include <fc/iostream.hpp>


FC_START_SHARED_IMPL(fc::http::connection)
   fc::tcp_socket sock;

   int read_until( char* buffer, char* end, char c = '\n' ) {
      char* p = buffer;
     // try {
          while( p < end && sock.read(p,1) ) {
            if( *p == c ) {
              *p = '\0';
              return (p - buffer)-1;
            }
            ++p;
          }
     // } catch ( ... ) {
     //   elog("%s", fc::current_exception().diagnostic_information().c_str() );
        //elog( "%s", fc::except_str().c_str() );
     // }
      return (p-buffer);
   }

   fc::http::reply parse_reply() {
      fc::http::reply rep;
      fc::vector<char> line(1024*8);
      int s = read_until( line.data(), line.data()+line.size(), ' ' ); // HTTP/1.1
      s = read_until( line.data(), line.data()+line.size(), ' ' ); // CODE
      rep.status = fc::lexical_cast<int>(fc::string(line.data()));
      s = read_until( line.data(), line.data()+line.size(), '\n' ); // DESCRIPTION

      while( (s = read_until( line.data(), line.data()+line.size(), '\n' )) > 1 ) {
        fc::http::header h;
        char* end = line.data();
        while( *end != ':' )++end;
        h.key = fc::string(line.data(),end);
        ++end; // skip ':'
        ++end; // skip space
        char* skey = end;
        while( *end != '\r' ) ++end;
        h.val = fc::string(skey,end);
        rep.headers.push_back(h);
        if( h.key == "Content-Length" ) {
           rep.body.resize( fc::lexical_cast<int>( fc::string(h.val) ) );
        }
      }
      if( rep.body.size() ) sock.read( rep.body.data(), rep.body.size() );
      return rep;
   }

FC_END_SHARED_IMPL 
#include <fc/shared_impl.cpp>



namespace fc { namespace http {

FC_REFERENCE_TYPE_IMPL( connection )


// used for clients
void       connection::connect_to( const fc::ip::endpoint& ep ) {
  my->sock.connect_to( ep );
}

http::reply connection::request( const fc::string& method, 
                                const fc::string& url, 
                                const fc::string& body ) {
	
  
  fc::stringstream req;
  req << method <<" "<<url<<" HTTP/1.1\r\n";
  req << "Host: localhost\r\n";
  if( body.size() ) req << "Content-Length: "<< body.size() << "\r\n";
  req << "\r\n"; 
  fc::string head = req.str();

  my->sock.write( head.c_str(), head.size() );

  if( body.size() )  {
      my->sock.write( body.c_str(), body.size() );
  }
  fc::cerr.flush();

  return my->parse_reply();
}

// used for servers
fc::tcp_socket& connection::get_socket() {
  return my->sock;
}

http::request    connection::read_request() {
  http::request r;
  return r;
}
void            connection::send_reply( const http::reply& ) {
  
}

} } // fc::http
