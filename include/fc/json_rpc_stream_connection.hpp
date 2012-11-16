#pragma once
#include <fc/json_rpc_connection.hpp>

namespace fc { 
  class istream;
  class ostream;

  namespace json {
  class rpc_stream_connection : public rpc_connection {
    public:
      typedef fc::shared_ptr<rpc_stream_connection> ptr;
      rpc_stream_connection( fc::istream&, fc::ostream& );
      rpc_stream_connection(const rpc_stream_connection& );
      rpc_stream_connection();

      // the life of the streams must exceed the life of all copies
      // of this rpc_stream_connection
      void open( fc::istream&, fc::ostream& );

      // cancels all pending requests, closes the ostream
      // results on_close() being called if the stream is not already closed.
      virtual void close();

      /**
       *  When the connection is closed, call the given method
       */
      void on_close( const std::function<void()>& );

    protected:
      ~rpc_stream_connection();
      virtual void send_invoke( uint64_t id, const fc::string& m, value&& param );
      virtual void send_notice(  const fc::string& m, value&& param );
      virtual void send_error( uint64_t id, int64_t code, const fc::string& msg );
      virtual void send_result( uint64_t id, value&& r );

    private:
      class impl;
      fc::shared_ptr<impl> my;
  };
} } // fc::json
