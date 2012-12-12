#pragma once 
#include <fc/string.hpp>
#include <fc/optional.hpp>
#include <fc/value.hpp>
#include <fc/reflect.hpp>


namespace fc { 
   class value;

   namespace json {

   class error_object {
      public:
         error_object( const fc::string& msg, const fc::value& v , int64_t c = -32000);
         error_object( const fc::string& msg=fc::string(), int64_t c = -32000 );
         error_object( const error_object& e );
         ~error_object();

         int64_t                 code;
         fc::string              message;
         fc::optional<fc::value> data;
   };

} }

FC_REFLECT( fc::json::error_object, (code)(message)(data) )
