#pragma once
#include <functional>
#include <memory>
#include <string>
#include <fc/any.hpp>

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
         virtual void close( int64_t code, const std::string& reason  ){};
         void on_message( const std::string& message ) { _on_message(message); }

         void on_message_handler( const std::function<void(const std::string&)>& h ) { _on_message = h; }

         void     set_session_data( fc::any d ){ _session_data = std::move(d); }
         fc::any& get_session_data() { return _session_data; }
      private:
         fc::any                                 _session_data; 
         std::function<void(const std::string&)> _on_message;
   };
   typedef std::shared_ptr<websocket_connection> websocket_connection_ptr;

   typedef std::function<void(const websocket_connection_ptr&)> on_connection_handler;

   class websocket_server 
   {
      public:
         websocket_server();
         ~websocket_server();

         void on_connection( const on_connection_handler& handler);
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

         websocket_connection_ptr connect( const std::string& uri );
      private:
         std::unique_ptr<detail::websocket_client_impl> my;
   };

} }
