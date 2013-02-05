#pragma once
#include <fc/value.hpp>
#include <fc/string.hpp>
#include <fc/time.hpp>
#include <fc/shared_ptr.hpp>


namespace fc  {

   struct log_level {
     enum   type {
        all, trace, debug, info, warn, error, fatal, off
     };
   };

   struct log_message {
      log_message(log_level::type, const string& file, int line, const string& func, const string& format );
      log_message();

      otime_point     when;
      log_level::type level;
      ostring         context;
      ostring         thread;
      ostring         fiber;
      string          file;
      int             line;
      string          method;
      string          format;
      value           args;
      ovalue          meta;   

      // key based args
      log_message& operator()( const string& arg, value&& v );
      log_message& operator()( const string& arg, const value& v );
      // position based args...
      log_message& operator()( value&& v );
      log_message& operator()( const value& v );
   };

   class appender;

   /**
    *
    *
    @code
      void my_class::func() 
      {
         fc_dlog( my_class_logger, "Format four: ${arg}  five: ${five}", ("arg",4)("five",5) );
      }
    @endcode
    */
   class logger {
      public:
         static logger get( const fc::string& name = "default");

         logger();
         logger( const string& name, const logger& parent = nullptr );
         logger( std::nullptr_t );
         logger( const logger& c );
         logger( logger&& c );
         ~logger();
         logger& operator=(const logger&);
         logger& operator=(logger&&);
         friend bool operator==( const logger&, std::nullptr_t );
         friend bool operator!=( const logger&, std::nullptr_t );

         logger& set_log_level( log_level::type e );
         log_level::type  get_log_level()const;
         logger& set_parent( const logger& l );
         logger  get_parent()const;

         void set_name( const fc::string& n );
         const fc::string& name()const;

         void add_appender( const fc::shared_ptr<appender>& a );


         bool is_enabled( log_level::type e )const;
         void log( log_message m );

      private:
         class impl;
         fc::shared_ptr<impl> my;
   };

   /**
    *  This helper class is used to automatically print a log message
    *  once upon construction, and again upon destruction and is therefore
    *  helpful in catching scope changes.
   struct tracer {
       tracer( const logger::ptr& lgr );
       ~tracer();

       void set_message( log_message&& ms g);

       private:
         logger::ptr logger;
         log_message msg;
   };
    */

} // namespace fc

#include <fc/reflect.hpp>
FC_REFLECT( fc::log_message, (when)(level)(context)(thread)(method)(file)(line)(format)(args)(meta) )
FC_REFLECT_ENUM( fc::log_level::type, (all)(trace)(debug)(info)(warn)(error)(fatal)(off) )

#define fc_scope_log( LOGGER, FORMAT, ... ) \
   fc::tracer __tracer; \
   if( (LOGGER).is_enabled( fc::log_level::trace ) ) { \
      __tracer.set_message( fc::log_message( fc::log_level::trace, __FILE__, __LINE__, __func__, FORMAT ) __VA_ARGS__ );\
   }
   
#define fc_dlog( LOGGER, FORMAT, ... ) \
   if( (LOGGER).is_enabled( fc::log_level::debug ) ) { \
      (LOGGER).log( fc::log_message( fc::log_level::debug, __FILE__, __LINE__, __func__, FORMAT ) __VA_ARGS__ );\
   }

#define fc_ilog( LOGGER, FORMAT, ... ) \
   if( (LOGGER).is_enabled( fc::log_level::info ) ) { \
      (LOGGER).log( fc::log_message( fc::log_level::info,  __FILE__, __LINE__, __func__, FORMAT ) __VA_ARGS__ );\
   }

#define fc_wlog( LOGGER, FORMAT, ... ) \
   if( (LOGGER).is_enabled( fc::log_level::warn ) ) { \
      (LOGGER).log( fc::log_message( fc::log_level::warn, __FILE__, __LINE__, __func__, FORMAT ) __VA_ARGS__ );\
   }

#define fc_elog( LOGGER, FORMAT, ... ) \
   if( (LOGGER).is_enabled( fc::log_level::error ) ) { \
      (LOGGER).log( fc::log_message( fc::log_level::error, __FILE__, __LINE__, __func__, FORMAT ) __VA_ARGS__ );\
   }

#define fc_flog( LOGGER, FORMAT, ... ) \
   if( (LOGGER).is_enabled( fc::log_level::fatal ) ) { \
      (LOGGER).log( fc::log_message( fc::log_level::fatal, __FILE__, __LINE__, __func__, FORMAT ) __VA_ARGS__ );\
   }

