#pragma once
#include <functional>
#include <memory>
#include <string>

namespace fc { namespace http {
   namespace detail {
      class websocket_server_impl;
      class websocket_client_impl;
   } // namespace detail;

   class websocket_connection
   {
      public:
         virtual ~websocket_connection(){};
         virtual void send_message( const std::string& message ) = 0;
   };
   typedef std::shared_ptr<websocket_connection> websocket_connection_ptr;

   class websocket_session
   {
      public:
         websocket_session( const websocket_connection_ptr& con )
         :_connection(con){}

         virtual ~websocket_session(){};
         virtual void on_message( const std::string& message ) = 0;

         void send_message( const std::string& message ) { _connection->send_message(message); }
      private:
         websocket_connection_ptr _connection;
   };
   typedef std::shared_ptr<websocket_session> websocket_session_ptr;

   typedef std::function< websocket_session_ptr( const websocket_connection_ptr& ) > session_factory;

   class websocket_server 
   {
      public:
         websocket_server();
         ~websocket_server();

         void on_connection( const session_factory& factory );
         void listen( uint16_t port );
         void start_accept();

      private:
         friend class detail::websocket_server_impl;
         std::unique_ptr<detail::websocket_server_impl> my;
   };

   class websocket_client
   {
      public:
         websocket_client();
         ~websocket_client();

         websocket_session_ptr connect( const std::string& uri, const session_factory& );
      private:
         std::unique_ptr<detail::websocket_client_impl> my;
   };

} }
