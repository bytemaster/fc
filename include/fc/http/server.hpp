#pragma once 
#include <fc/http/connection.hpp>
#include <fc/shared_ptr.hpp>
#include <functional>

namespace fc { namespace http {

  /**
   *  Listens on a given port for incomming http
   *  connections and then calls a user provided callback
   *  function for every http request.
   *
   */
  class server {
    public:
      server();
      server( uint16_t port );
      server( const server& s );
      server( server&& s );
      ~server();

      server& operator=(const server& s);
      server& operator=(server&& s);

      class response {
        public:
          class impl;

          response();
          response( const fc::shared_ptr<impl>& my);
          response( const response& r);
          response( response&& r );
          ~response();
          response& operator=(const response& );
          response& operator=( response&& );

          void add_header( const fc::string& key, const fc::string& val )const;
          void set_status( const http::reply::status_code& s )const;
          void set_length( uint64_t s )const;

          void write( const char* data, uint64_t len )const;

        private:
          fc::shared_ptr<impl> my;
      };

      void listen( uint16_t p );

      /**
       *  Set the callback to be called for every http request made.
       */
      void on_request( const std::function<void(const http::request&, const server::response& s )>& cb );

    private:
      class impl;
      fc::shared_ptr<impl> my;

  };

} }
