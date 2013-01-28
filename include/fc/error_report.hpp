#pragma once
#include <fc/vector.hpp>
#include <fc/string.hpp>
#include <fc/value.hpp>
#include <fc/optional.hpp>
#include <fc/exception.hpp>

namespace fc {

   /**
    * Represents one stack frame within an error_report.
    */
   class error_frame {
      public:
         error_frame( const fc::string& file, uint64_t line, const fc::string& method, const fc::string& desc, fc::value m );
         error_frame():file("unknown-file"),line(0){}
         error_frame(const error_frame& );
         error_frame(error_frame&& );

         error_frame& operator=(const error_frame& );
         error_frame& operator=(error_frame&& );

         fc::string    to_string()const;
         fc::string    to_detail_string()const;

         fc::string                  desc;
         fc::string                  file;
         int64_t                     line;
         fc::string                  method;
         fc::string                  time;
         fc::optional<fc::value>     meta;
   };
   typedef fc::vector<error_frame> error_context;

   /**
    *  This class is used for rich error reporting that captures relevant errors the
    *  whole way up the stack.  By using FC_THROW_REPORT(...) and FC_REPORT_PUSH( e, ...)
    *  you can capture the file, line, and method where the error was caught / rethrown.
    */
   class error_report {
      public:
         error_report();
         error_report( const fc::string& file, uint64_t line, const fc::string& method, const fc::string& desc, fc::value meta = fc::value() );

         error_frame&  current();
         error_report& pop_frame();
         error_report& push_frame( const fc::string& file, uint64_t line, const fc::string& method, const fc::string& desc, fc::value meta = fc::value() );
         error_report& append( const error_report& e );

         fc::string    to_string()const;
         fc::string    to_detail_string()const;
         error_context stack;    ///< Human readable stack of what we were atempting to do.

         fc::exception_ptr copy_exception();
   };

   fc::string substitute( const fc::string& format, const fc::value& keys );
   fc::value  recursive_substitute( const value& in, const fc::value& keys );

} // namespace fc

#include <fc/reflect.hpp>
FC_REFLECT( fc::error_frame,  (desc)(file)(line)(method)(time)(meta) )
FC_REFLECT( fc::error_report, (stack) )

#define FC_REPORT( X, ... )         fc::error_report X( __FILE__, __LINE__, __func__, __VA_ARGS__ )
#define FC_THROW_REPORT( ... )      FC_THROW( fc::error_report( __FILE__, __LINE__, __func__, __VA_ARGS__ ))
#define FC_REPORT_CURRENT(ER, ... ) (ER).pop_frame().push_frame( __FILE__, __LINE__, __func__, __VA_ARGS__ )
#define FC_REPORT_PUSH( ER, ... )   (ER).push_frame( __FILE__, __LINE__, __func__, __VA_ARGS__ );
#define FC_REPORT_POP(ER)           (ER).pop_frame()
