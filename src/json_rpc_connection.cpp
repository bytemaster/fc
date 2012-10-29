#include <fc/json_rpc_connection.hpp>
#include <fc/log.hpp>
#include <fc/thread.hpp>
#include <fc/error.hpp>
#include <unordered_map>
#include <string>

namespace fc { namespace json {

  namespace detail {

      void pending_result::handle_error( const fc::string& e ) {
         try {
          FC_THROW_MSG( "%s", e );
         } catch ( ... ) {
          set_exception( fc::current_exception() );
         }
      }

  }

  class rpc_connection::impl : public fc::retainable {
    public:
      impl():_next_req_id(0){ }
      ~impl(){ cancel_pending_requests();  }
      int64_t                                                    _next_req_id;
      std::unordered_map<std::string,detail::server_method::ptr> _methods;

      detail::pending_result::ptr _pr_head;
      detail::pending_result::ptr _pr_tail;

      void cancel_pending_requests() {
         auto cur = _pr_head;
         while( cur  ) {
           cur->set_exception( fc::copy_exception( fc::generic_exception("canceled") ) );
           cur = cur->next;
         }
         _pr_head.reset();
         _pr_tail.reset();
      }
  };


  void rpc_connection::handle_message( const value& v ) {
     auto id_itr = v.find( "id" );
     auto m_itr  = v.find( "method" );
     auto end    = v.end();

     if( m_itr != end ) {
        fc::string mname = value_cast<fc::string>(m_itr->val);
        auto id = value_cast<uint64_t>(id_itr->val);
        auto smeth = my->_methods.find( mname );
        if( smeth == my->_methods.end() ) {
          if( id_itr != end )  {
            // TODO: send invalid method reply
            send_error( id, -1, "Unknown method '"+mname+"'");
          }
          // nothing to do, unknown method
        } else { // known method, attempt to call it and send reply;
          auto p_itr = v.find( "params" );

          value nul;
          const value& params = (p_itr != end) ? p_itr->val : nul;

          if( id_itr != end ) { // capture reply
            try {
               send_result( id, smeth->second->call(params) );
            } catch ( ... ) {
               send_error( id, -1, fc::except_str() );
            }
          } else { // ignore exception + result
            try { smeth->second->call( params ); }catch(...){}
          }
        }
        return;
     } else if( id_itr != end ) { // we id but no method, therefore potential reply
        int id = value_cast<int64_t>(id_itr->val);
        auto cur = my->_pr_head;
        decltype(cur) prev;
        while( cur ) {
            if( cur->id == id ) {
              if( prev ) prev->next = cur->next;
              else my->_pr_head = cur->next;
              if( !cur->next ) my->_pr_tail = cur->next;
              
              try {
                  auto r_itr = v.find( "result" );
                  if( r_itr != end ) {
                    cur->handle_result( r_itr->val );
                  } else {
                    auto e_itr = v.find( "error" );
                    if( e_itr != end ) {
                      cur->set_exception( 
                        fc::copy_exception( 
                          fc::generic_exception( 
                            value_cast<fc::string>(
                              e_itr->val["message"] ) ) ) );
                    }
                  }
              } catch( ... ) {
                cur->set_exception( fc::current_exception() );
              }
              return;
            }
            cur = cur->next;
        }
        FC_THROW_MSG( "Unexpected reply with id %s", id );
     }
     FC_THROW_MSG( "Method with no 'id' or 'method' field" );
  }


  rpc_connection::rpc_connection()
  :my( new impl() ){ }

  rpc_connection::rpc_connection( const rpc_connection& c )
  :my(c.my){ }
  rpc_connection::rpc_connection( rpc_connection&& c ) {
    fc::swap(my,c.my);
  }
  rpc_connection::~rpc_connection() {
  }

  rpc_connection& rpc_connection::operator=(const rpc_connection& m) {
    my= m.my;
    return *this;
  }
  rpc_connection& rpc_connection::operator=(rpc_connection&& m) {
    fc::swap(m.my,my);
    return *this;
  }

  void rpc_connection::cancel_pending_requests() {
    my->cancel_pending_requests();
  }

  void rpc_connection::invoke( detail::pending_result::ptr&& p, 
                               const fc::string& m, value&& param ) {
    if( my->_pr_tail ) {
      my->_pr_tail->next = p;
      my->_pr_tail = my->_pr_tail->next;
    } else {
      my->_pr_tail = p;
      my->_pr_head = my->_pr_tail;
    }
    send_invoke( ++my->_next_req_id, m, fc::move(param) );
  }


} } // fc::json
