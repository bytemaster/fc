#pragma once
#include <fc/variant.hpp>

namespace fc
{
   class path;
   class ostream;
   class buffered_istream;

   /**
    *  Provides interface for json serialization.
    *
    *  json strings are always UTF8
    */
   class json
   {
      public:
         static ostream& to_stream( ostream& out, const fc::string& );
         static ostream& to_stream( ostream& out, const variant& v );
         static ostream& to_stream( ostream& out, const variants& v );
         static ostream& to_stream( ostream& out, const variant_object& v );

         static variant  from_stream( buffered_istream& in );

         static variant  from_string( const string& utf8_str );
         static string   to_string( const variant& v );
         static string   to_pretty_string( const variant& v );

         template<typename T>
         static void     save_to_file( const T& v, const fc::path& fi, bool pretty = true )
         {
            save_to_file( variant(v), fi, pretty );
         }

         static void     save_to_file( const variant& v, const fc::path& fi, bool pretty = true );
         static variant  from_file( const fc::path& p );

         template<typename T>
         static T from_file( const fc::path& p )
         {
            return json::from_file(p).as<T>();
         }

         template<typename T>
         static string   to_string( const T& v ) 
         {
            return to_string( variant(v) );
         }

         template<typename T>
         static string   to_pretty_string( const T& v ) 
         {
            return to_pretty_string( variant(v) );
         }

         template<typename T>
         static void save_to_file( const T& v, const string& p, bool pretty = true ) 
         {
            save_to_file( variant(v), p, pretty );
         } 
   };

} // fc
