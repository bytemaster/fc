#pragma once 

namespace fc { namespace json {

   struct error_object {
      error_object( const fc::string& msg = fc::string(), fc::value v = fc::value(), int64_t c = -32000);

      int64_t                 code;
      fc::string              message;
      fc::optional<fc::value> data;
   };

} }

#include <fc/reflect.hpp>
FC_REFLECT( fc::json::error_object, (code)(message)(data) )
