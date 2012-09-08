#ifndef _FC_JSON_HPP_
#define _FC_JSON_HPP_
#include <fc/string.hpp>
#include <fc/reflect_fwd.hpp>
#include <fc/value_fwd.hpp>

namespace fc { 
   class istream;
   class ostream;
   class cref;

   namespace json {
     string to_string( const cref& o );
     value_fwd from_string( const string& s );
     value_fwd from_string( const char* s, const char* e );
     void      from_string( const string&, const ref& o );

     template<typename T>
     T  from_string( const string& s ) {
        T tmp; from_string( s, tmp );
        return tmp;
     }
     string escape_string( const string& );
     string unescape_string( const string& );
     void write( ostream& out, const cref& val );
} }

#endif 
