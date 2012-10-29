#include <fc/json_rpc_stream_connection.hpp>
#include <fc/iostream.hpp>
#include <fc/thread.hpp>

namespace fc { namespace json {

  class rpc_stream_connection::impl : public fc::retainable {
    public:
      fc::istream&           in;
      fc::ostream&           out;
      rpc_stream_connection& self;
      fc::function<void>     on_close;

      impl( fc::istream& i, fc::ostream& o, rpc_stream_connection& s )
      :in(i),out(o),self(s){
        _read_loop_complete = fc::async( [=](){ read_loop(); } ); 
      }
      
      ~impl() {
        try {
            self.cancel_pending_requests();
            _read_loop_complete.cancel();
            _read_loop_complete.wait();
        } catch ( ... ) {}
      }

      fc::future<void> _read_loop_complete;
      void read_loop() {
        fc::string line;
        fc::getline( in, line );
        while( !in.eof() ) {
            try {
                fc::value v= fc::json::from_string( line );

            } catch (...) {
              wlog( "%s", fc::except_str().c_str() );
            }
            fc::getline( in, line );
        }
        self.cancel_pending_requests();
        if( !!on_close ) on_close();
      }
  };

  rpc_stream_connection::rpc_stream_connection( fc::istream& i, fc::ostream& o )
  :my( new impl(i,o,*this) ){
  }

  // the life of the streams must exceed the life of all copies
  // of this rpc_stream_connection
  void rpc_stream_connection::open( fc::istream& i, fc::ostream& o) {
    my.reset( new impl(i,o,*this) );
  }

  // cancels all pending requests, closes the ostream
  // results on_close() being called if the stream is not already closed.
  void rpc_stream_connection::close() {
    my->out.close();
  }

  /**
   *  When the connection is closed, call the given method
   */
  void rpc_stream_connection::on_close( const fc::function<void>& oc ) {
    my->on_close = oc;
  }

  void rpc_stream_connection::send_invoke( uint64_t id, const fc::string& m, value&& param ) {
  }
  void rpc_stream_connection::send_error( uint64_t id, int64_t code, const fc::string& msg ) {
  }
  void rpc_stream_connection::send_result( uint64_t id, value&& r ) {
  }

} } // fc::json



