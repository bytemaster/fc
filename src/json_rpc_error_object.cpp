#include <fc/json_rpc_error_object.hpp>

namespace fc { namespace json {
   error_object::error_object( const fc::string& m, fc::value v, int64_t c )
   :code(c),message(m),data(fc::move(v)){}
}}
