#ifndef _JSON_RPC_CONNECTION_HPP_
#define _JSON_RPC_CONNECTION_HPP_
#include <fc/reflect_ptr.hpp>
#include <fc/json.hpp>
#include <fc/stream.hpp>
#include <fc/future.hpp>

namespace fc {  namespace json {
  namespace detail {
    struct pending_result : virtual public promise_base {
      typedef shared_ptr<pending_result> ptr;
      virtual void handle_result( const fc::string& ) = 0;
      void handle_error( const fc::string& );
      int64_t             id;
      pending_result::ptr next;
      protected:
        ~pending_result(){}
    };
    template<typename T>
    struct pending_result_impl : virtual public promise<T>, virtual public pending_result {   
       virtual void handle_result( const fc::string& s ) {
          set_value( fc::json::from_string<T>(s) );
       }
      protected:
        ~pending_result_impl(){}
    };
    template<>
    struct pending_result_impl<void> : virtual public promise<void>, virtual public pending_result {   
       virtual void handle_result( const fc::string& ) {
          set_value();
       }
      protected:
        ~pending_result_impl(){}
    };
  }

  /**
    This class is designed to be used like this:
   @code
     class my_api {
        
       future<int> function( const string& arg, int arg2 ) {
          _con->invoke<int>( "function", {&arg,&arg2} ); 
       }
       private:
         rpc_connection* _con;
     };
   @endcode
    
   */
  class rpc_connection : virtual public retainable {
    public:
      rpc_connection();
      rpc_connection( istream& i, ostream& o );
      rpc_connection( rpc_connection&& c );
      ~rpc_connection();

      rpc_connection& operator=(rpc_connection&& m);

      void init( istream& i, ostream& o );

      template<typename R, uint16_t N>
      future<R> invoke( const fc::string& method, const cptr (&params)[N] ) {
        auto r = new detail::pending_result_impl<R>();
        invoke( detail::pending_result::ptr(r), method, N, params );
        return promise<R>::ptr( r, true );
      }

    private:
      void invoke( detail::pending_result::ptr&& p, const fc::string& m, 
                   uint16_t nparam, const cptr* param );
      class rpc_connection_d* my;
  };
} } // fc::json


#endif // _JSON_RPC_CONNECTION_HPP_
