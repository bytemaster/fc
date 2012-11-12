#include <fc/json_rpc_error_object.hpp>

namespace fc { namespace json {
   error_object::error_object( const fc::string& msg, fc::value v, int c )
   :code(c),message(m),data(fc::move(v)){}
}}
