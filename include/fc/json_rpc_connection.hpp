#pragma once
#include <fc/json.hpp>
#include <fc/iostream.hpp>
#include <fc/future.hpp>
#include <fc/function.hpp>

namespace fc {  namespace json {
  namespace detail {
    struct pending_result : virtual public promise_base {
      typedef shared_ptr<pending_result> ptr;
      virtual void handle_result( const fc::value& ) = 0;
      void handle_error( const fc::string& );
      int64_t             id;
      pending_result::ptr next;
      protected:
        ~pending_result(){}
    };
    template<typename T>
    struct pending_result_impl : virtual public promise<T>, virtual public pending_result {   
       virtual void handle_result( const fc::value& s ) {
          this->set_value( value_cast<T>(s) );
       }
      protected:
        ~pending_result_impl(){}
    };
    template<>
    struct pending_result_impl<void> : virtual public promise<void>, virtual public pending_result {   
       virtual void handle_result( const fc::value& ) {
          set_value();
       }
      protected:
        ~pending_result_impl(){}
    };
  }

  /**
   *  This class can be used to communicate via json-rpc over a pair of
   *  streams.
   *
   *  @note rpc_connection has reference semantics and all 'copies' will
   *  refer to the same underlying stream.
   */
  class rpc_connection {
    public:
      rpc_connection();
      /** note the life of i and o must be longer than rpc_connection's life */
      rpc_connection( istream& i, ostream& o );
      rpc_connection( rpc_connection&& c );
      rpc_connection( const rpc_connection& c );
      ~rpc_connection();

      rpc_connection& operator=(rpc_connection&& m);
      rpc_connection& operator=(const rpc_connection& m);

      /** note the life of i and o must be longer than rpc_connection's life */
      void init( istream& i, ostream& o );

      /*
      template<typename R >
      future<R> invoke( const fc::string& method  ) {
        auto r = new detail::pending_result_impl<R>();
        invoke( detail::pending_result::ptr(r), method, value(make_tuple()) );
        return promise<R>::ptr( r, true );
      } */

      template<typename R, typename Args >
      future<R> invoke( const fc::string& method, Args&& a = nullptr  )const {
        auto r = new detail::pending_result_impl<R>();
        slog( "%p", r );
        typename promise<R>::ptr rtn( r, true );
        invoke( detail::pending_result::ptr(r), method, value(fc::forward<Args>(a)) );
        return rtn;
      }

    private:
      void invoke( detail::pending_result::ptr&& p, const fc::string& m, const value& param )const;
      class impl;
      fc::shared_ptr<class impl> my;
  };




} } // fc::json


