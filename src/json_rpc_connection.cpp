#include <fc/json_rpc_connection.hpp>

namespace fc { namespace json {

  class rpc_connection_d {
    public:
      rpc_connection_d()
      :_in(0),_out(0),_next_req_id(0){}
      istream* _in;
      ostream* _out;
      int64_t  _next_req_id;
      detail::pending_result::ptr _pr_head;
      detail::pending_result::ptr _pr_tail;
  };
  rpc_connection::rpc_connection() {
    my = new rpc_connection_d();  
  }
  rpc_connection::rpc_connection( istream& i, ostream& o ) {
    my = new rpc_connection_d();  
    init( i, o );
  }
  rpc_connection::rpc_connection( rpc_connection&& c ) 
  :my(c.my) {
    c.my = 0;
  }
  rpc_connection::~rpc_connection() {
    delete my;
  }

  rpc_connection& rpc_connection::operator=(rpc_connection&& m) {
    fc::swap(m.my,my);
    return *this;
  }

  void rpc_connection::init( istream& i, ostream& o ) {
    my->_in  = &i;
    my->_out = &o;
  }
  void rpc_connection::invoke( detail::pending_result::ptr&& p, const fc::string& m, 
                                    uint16_t nparam, const cptr* param ) {
    p->id = ++my->_next_req_id;

    my->_pr_tail->next = fc::move(p);
    my->_pr_tail = my->_pr_tail->next;

    ostream& out = *my->_out;
    out << "{\"id\":"<<p->id<<",\"method\":"<<escape_string(m);
    if( nparam > 0 ) {
      out <<",\"params\":[";
      uint16_t back = nparam -1;
      for( uint16_t i = 0; i < back; ++i ) {
        fc::json::write( out, *(param[i]) );
        out <<',';
      }
      fc::json::write( out, *(param[back]) );
      out<<']';
    }
    out<<"}\n";
    out.flush();
  }


} } // fc::json
