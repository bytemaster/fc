#include <fc/network/udt_socket.hpp>

namespace fc {

   void check_udt_errors()
   {
      UDT::ERRORINFO& error_info = UDT::getlasterror();
      if( error_info.getErrorCode() )
      {
         std::string  error_message = error_info.getErrorMessage();
         error_info.clear();
         FC_CAPTURE_AND_THROW( udt_exception, (error_message) );
      }
   }

   udt_socket::udt_socket()
   :_udt_socket_id( UDT::INVALID_SOCK )
   {
   }

   ~udt_socket()
   {
      close();
   }

   void udt_socket::connect_to( const ip::endpoint& remote_endpoint )
   { try {
      sockaddr_in serv_addr;
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(remote_endpoint.port());
      serv_addr.sin_addr = htonl(remote_endpoint.address());

      // connect to the server, implict bind
      if (UDT::ERROR == UDT::connect(_udt_socket_id, (sockaddr*)&serv_addr, sizeof(serv_addr)))
         check_udt_errors();

   } FC_CAPTURE_AND_RETHROW( (remote_endpoint) ) }

   ip::endpoint udt_socket::remote_endpoint() const
   { try {
      sockaddr_in peer_addr;
      int peer_addr_size = sizeof(peer_addr);
      int error_code = UDT::getpeername( _udt_socket_id, &peer_addr, &peer_addr_size );
      if( error_code == UDT::ERROR )
          check_udt_errors();
      return ip::endpoint( address( htonl( peer_addr.sin_addr ) ), htons(peer_addr.sin_port) );
   } FC_CAPTURE_AND_RETHROW() }

   ip::endpoint udt_socket::local_endpoint() const
   { try {
      sockaddr_in sock_addr;
      int addr_size = sizeof(peer_addr);
      int error_code = UDT::getsockname( _udt_socket_id, &sock_addr, &addr_size );
      if( error_code == UDT::ERROR )
          check_udt_errors();
      return ip::endpoint( address( htonl( sock_addr.sin_addr ) ), htons(sock_addr.sin_port) );
   } FC_CAPTURE_AND_RETHROW() }


   /// @{
   size_t   udt_socket::readsome( char* buffer, size_t max )
   { try {
      auto bytes_read = UDT::recv( _udt_socket_id, buffer, max, 0 );
      if( bytes_read == UDT::ERROR )
      {
         if( UDT::getlasterror().getCode() == UDT::EASYNCRCV )
         {
            // create a future and post to epoll, wait on it, then
            // call readsome recursively.
         }
         else
            check_udt_errors();
      }
      return bytes_read;
   } FC_CAPTURE_AND_RETHROW( (max) ) }

   bool     udt_socket::eof()const
   {
      // TODO... 
      return false;
   }
   /// @}
   
   /// ostream interface
   /// @{
   size_t   udt_socket::writesome( const char* buffer, size_t len )
   {
      auto bytes_sent = UDT::send(_udt_socket_idl, buffer, len, 0);

      if( UDT::ERROR == bytes_sent )
         check_udt_errors();

      if( bytes_sent == 0 )
      {
         // schedule wait with epoll 
      }
      return bytes_sent;
   }

   void     udt_socket::flush(){}

   void     udt_socket::close()
   { try {
      UDT::close( _udt_socket_id );
      check_udt_errors();
   } FC_CAPTURE_AND_RETHROW() }
   /// @}
   
   void udt_socket::open()
   {
      _udt_socket_id = UDT::socket(AF_INET, SOCK_STREAM, 0);
   }

   bool udt_socket::is_open()const
   {
      return _udt_socket_id != UDT::INVALID_SOCK;
   }
     

} 
