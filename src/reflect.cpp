#include <fc/error_report.hpp>

namespace fc {
   void throw_bad_enum_cast( int64_t i, const char* e ) {
        FC_THROW_REPORT(  "Unknown field ${field} not in enum ${enum}", 
                          fc::value().set("field",i).set("enum",e) ); 
   }
   void throw_bad_enum_cast( const char* k, const char* e ){
        FC_THROW_REPORT(  "Field '${field}' not in enum ${enum}", 
                          fc::value().set("field",k).set("enum",e) ); 
   }
}
