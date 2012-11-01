#pragma once
#include <fc/json_rpc_client.hpp>
#include <fc/process.hpp>

namespace fc { namespace json {

  template<typename InterfaceType>
  class rpc_process_client : public ptr<InterfaceType,fc::json::detail::rpc_member> {
    public:

      fc::future<int> exec( const fc::path& exe, int opt = fc::process::open_all ) {
        return exec( exe, fc::path(), opt );
      }
      fc::future<int> exec( const fc::path& exe, const fc::path& wd, 
                                  int opt = fc::process::open_all ) {
        return exec( exe, fc::vector<fc::string>(), wd, opt );
      }
      fc::future<int> exec( const fc::path& exe, fc::vector<fc::string>&& args , 
                            int opt = fc::process::open_all ) {
        return exec( exe, fc::move(args), fc::path(), opt );
      }
      fc::future<int> exec( const fc::path& exe, fc::vector<fc::string>&& args, 
                            const fc::path& wd, int opt = fc::process::open_all  ) {
         auto r = _proc.exec( exe, fc::move(args), wd, opt ); 
         _con.reset( new fc::json::rpc_stream_connection( _proc.out_stream(), _proc.in_stream() ) );
         this->_vtable.reset(new fc::detail::vtable<InterfaceType,fc::json::detail::rpc_member>() );
         this->_vtable->template visit<InterfaceType>( fc::json::detail::vtable_visitor(_con) );
         return r;
      }

      void kill() { _con->close(); _proc.kill(); }

      /**
       *  @brief returns a stream that reads from the process' stderr
       */
      fc::istream& err_stream() { return _proc.err_stream(); }

      template<typename T&&>
      void on_close( T&& f) { _con->on_close( fc::forward<T>(f) ); }

    private:
      fc::process                    _proc;
      fc::json::rpc_stream_connection::ptr _con;
  };
} }
