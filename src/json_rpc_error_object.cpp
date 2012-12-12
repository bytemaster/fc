#include <fc/json_rpc_error_object.hpp>
#include <fc/json.hpp>


namespace fc { namespace json {
   error_object::error_object( const fc::string& m, const fc::value& v, int64_t c )
   :code(c),message(m),data(v){ }
   error_object::error_object(const fc::string& m, int64_t c)
   :code(c),message(m){ }
   error_object::error_object(const error_object& e)
   :code(e.code),message(e.message),data(e.data){}
   error_object::~error_object(){}
}}
