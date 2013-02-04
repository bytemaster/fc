#include <fc/json_rpc_stream_connection.hpp>
#include <fc/json_rpc_error_object.hpp>
#include <fc/iostream.hpp>
#include <fc/sstream.hpp>
#include <fc/thread.hpp>

#include <iostream>

namespace fc { namespace json {

  class rpc_stream_connection::impl : public fc::retainable {
    public:
      fc::istream&           in;
      fc::ostream&           out;
      rpc_stream_connection& self;
      std::function<void()>  on_close;

      impl( fc::istream& i, fc::ostream& o, rpc_stream_connection& s )
      :in(i),out(o),self(s){
        _read_loop_complete = fc::async( [=](){ read_loop(); } ); 
      }
      
      ~impl() {
        try {
            self.cancel_pending_requests();
            self.close();
            _read_loop_complete.cancel();
            _read_loop_complete.wait();
        } catch ( ... ) {}
      }

      fc::future<void> _read_loop_complete;
      void read_loop() {
         try {
           fc::string line;
           fc::getline( in, line );
           while( !in.eof() ) {
	    // std::cerr<<"\n**line size: "<<line.size()<<"\n\n";
            //   slog( "line size: '%d'", line.size() );
            //   slog( "%s", line.c_str() );
               try {
                 fc::value v= fc::json::from_string( line );
                 self.handle_message(v);
               } catch (...) {
                 wlog( "%s", fc::except_str().c_str() );
                 return;
               }
               fc::getline( in, line );
           }
         } catch ( ... ) {
             wlog( "%s", fc::except_str().c_str() );
         }
      //   slog( "cancel...");
         self.cancel_pending_requests();
       //  slog( "close!" );
         if( !!on_close ) on_close();
      }
  };

  rpc_stream_connection::rpc_stream_connection( fc::istream& i, fc::ostream& o )
  :my( new impl(i,o,*this) ){
  }
  rpc_stream_connection::rpc_stream_connection(){  }
  rpc_stream_connection::rpc_stream_connection(const rpc_stream_connection& c):my(c.my){ }
  rpc_stream_connection::~rpc_stream_connection(){ 
    wlog( "%p", this );
  }

  // the life of the streams must exceed the life of all copies
  // of this rpc_stream_connection
  void rpc_stream_connection::open( fc::istream& i, fc::ostream& o) {
    my.reset( new impl(i,o,*this) );
  }

  // cancels all pending requests, closes the ostream
  // results on_close() being called if the stream is not already closed.
  void rpc_stream_connection::close() {
    if( my ) my->out.close();
    my.reset(nullptr);
  }

  /**
   *  When the connection is closed, call the given method
   */
  void rpc_stream_connection::on_close( const std::function<void()>& oc ) {
    my->on_close = oc;
  }

  void rpc_stream_connection::send_invoke( uint64_t id, const fc::string& m, value&& param ) {
    fc::stringstream ss;
    ss<<"{\"id\":"<<id<<",\"method\":\""<<m<<"\",\"params\":"<<fc::json::to_string(param)<<"}\n";
    fc::string o = ss.str();
    my->out.write( o.c_str(), o.size() );
    my->out.flush();
  }
  void rpc_stream_connection::send_notice( const fc::string& m, value&& param ) {
    fc::stringstream ss;
    ss<<"{\"method\":\""<<m<<"\",\"params\":"<<fc::json::to_string(param)<<"}\n";
    fc::string o = ss.str();
    my->out.write( o.c_str(), o.size() );
    my->out.flush();
  }
  void rpc_stream_connection::send_error( uint64_t id, const json::error_object& eo ) {
    fc::stringstream ss;
    ss<<"{\"id\":"<<id<<",\"error\":"<<fc::json::to_string(eo)<<"}\n";
    fc::string o = ss.str();
    my->out.write( o.c_str(), o.size() );
    my->out.flush();
  }
  void rpc_stream_connection::send_result( uint64_t id, value&& r ) {
    fc::stringstream ss;
    ss<<"{\"id\":"<<id<<",\"result\":"<<fc::json::to_string(r)<<"}\n";
    fc::string o = ss.str();
    my->out.write( o.c_str(), o.size() );
    my->out.flush();
  }

} } // fc::json



