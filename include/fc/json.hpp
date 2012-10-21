#pragma once
#include <fc/string.hpp>
#include <fc/value.hpp>
#include <fc/value_cast.hpp>
#include <fc/vector_fwd.hpp>

namespace fc { 
   class istream;
   class ostream;
   class path;

   namespace json {
     string to_string( const value& o );


     value from_string( const string& s );
     value from_string( const char* s, const char* e );
     value from_string( const fc::vector<char>& v );


     string escape_string( const string& );
     string unescape_string( const string& );

     void write( ostream& out, const value& val );

     template<typename T>
     void write( ostream& out, const T& val ) {
        write( out, value(val) );
     }

     template<typename T>
     string to_string( const T& o ) { 
        return json::to_string(value(o)); 
     }

     template<typename T>
     T  from_string( const string& s ) {
        return value_cast<T>( from_string(s) );
     }

     value from_file( const fc::path& s );
     template<typename T>
     T  from_file( const fc::path& s ) {
        return value_cast<T>( fc::json::from_file(s) );
     }
  } // namespace json 
} // fc

