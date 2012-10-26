#include <fc/json_rpc_connection.hpp>
#include <fc/log.hpp>
#include <fc/thread.hpp>

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
      impl()
      :_in(0),_out(0),_next_req_id(0){ }
      ~impl(){  }
      istream* _in;
      ostream* _out;
      int64_t  _next_req_id;
      detail::pending_result::ptr _pr_head;
      detail::pending_result::ptr _pr_tail;

      fc::future<void> _read_loop_complete;
      void read_loop() {
        fc::string line;
        fc::getline( *_in, line );
        while( !_in->eof() ) {
            try {
                fc::value v= fc::json::from_string( line );

                auto id_itr = v.find( "id" );
                auto result_itr = v.find( "result" );
                if( id_itr != v.end() && result_itr != v.end() ) {
                    int id = value_cast<int64_t>(id_itr->val);
                    auto cur = _pr_head;
                    decltype(cur) prev;
                    while( cur ) {
                        if( cur->id == id ) {
                          if( prev ) prev->next = cur->next;
                          else _pr_head = cur->next;
                          if( !cur->next ) _pr_tail = cur->next;

                          cur->handle_result( result_itr->val );
                          break;
                        }
                        cur = cur->next;
                    }
                }
            } catch (...) {
              wlog( "%s", fc::except_str().c_str() );
            }
            fc::getline( *_in, line );
        }
        slog( "Exit Read Loop, canceling waiting tasks!" );

        auto cur = _pr_head;
        while( cur ) {
          cur->handle_error( "Connection Closed" );
          cur = cur->next;
        }

        _pr_head.reset();
        _pr_tail.reset();
      }
  };
  rpc_connection::rpc_connection()
  :my( new impl() ){ }
  rpc_connection::rpc_connection( istream& i, ostream& o )
  :my( new impl() )
  {
    init( i, o );
  }
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

  void rpc_connection::init( istream& i, ostream& o ) {
    my->_in  = &i;
    my->_out = &o;
    my->_read_loop_complete = fc::async( [=](){ my->read_loop(); } );
  }
  void rpc_connection::invoke( detail::pending_result::ptr&& p, 
                               const fc::string& m, const value& v )const {
                                
    p->id = ++my->_next_req_id;

    if( my->_pr_tail ) {
      my->_pr_tail->next = p;
      my->_pr_tail = my->_pr_tail->next;
    } else {
      my->_pr_tail = p;
      my->_pr_head = my->_pr_tail;
    }

    ostream& out = *my->_out;
    out << "{\"id\":"<<p->id<<",\"method\":\""<<escape_string(m);
    out <<"\",\"params\":";
    fc::json::write( out, v ); 
    out<<"}\n";
    out.flush();
  }


} } // fc::json
