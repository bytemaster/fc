#pragma once
#include <fc/string.hpp>
#include <fc/time.hpp>
#include <fc/shared_ptr.hpp>
#include <fc/log/log_message.hpp>

namespace fc  
{

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
   class logger 
   {
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
         friend bool operator==( const logger&, nullptr_t );
         friend bool operator!=( const logger&, nullptr_t );

         logger&    set_log_level( log_level e );
         log_level  get_log_level()const;
         logger&    set_parent( const logger& l );
         logger     get_parent()const;

         void  set_name( const fc::string& n );
         const fc::string& name()const;

         void add_appender( const fc::shared_ptr<appender>& a );

         bool is_enabled( log_level e )const;
         void log( log_message m );

      private:
         class impl;
         fc::shared_ptr<impl> my;
   };

} // namespace fc

   
#define fc_dlog( LOGGER, FORMAT, ... ) \
  do { \
   if( (LOGGER).is_enabled( fc::log_level::debug ) ) { \
      (LOGGER).log( FC_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
   } \
  } while (0)

#define fc_ilog( LOGGER, FORMAT, ... ) \
  do { \
   if( (LOGGER).is_enabled( fc::log_level::info ) ) { \
      (LOGGER).log( FC_LOG_MESSAGE( info, FORMAT, __VA_ARGS__ ) ); \
   } \
  } while (0)

#define fc_wlog( LOGGER, FORMAT, ... ) \
  do { \
   if( (LOGGER).is_enabled( fc::log_level::warn ) ) { \
      (LOGGER).log( FC_LOG_MESSAGE( warn, FORMAT, __VA_ARGS__ ) ); \
   } \
  } while (0)

#define fc_elog( LOGGER, FORMAT, ... ) \
  do { \
   if( (LOGGER).is_enabled( fc::log_level::error ) ) { \
      (LOGGER).log( FC_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
   } \
  } while (0)

#define dlog( FORMAT, ... ) \
  do { \
   if( (fc::logger::get()).is_enabled( fc::log_level::debug ) ) { \
      (fc::logger::get()).log( FC_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
   } \
  } while (0)

#define ilog( FORMAT, ... ) \
  do { \
   if( (fc::logger::get()).is_enabled( fc::log_level::info ) ) { \
      (fc::logger::get()).log( FC_LOG_MESSAGE( info, FORMAT, __VA_ARGS__ ) ); \
   } \
  } while (0)

#define wlog( FORMAT, ... ) \
  do { \
   if( (fc::logger::get()).is_enabled( fc::log_level::warn ) ) { \
      (fc::logger::get()).log( FC_LOG_MESSAGE( warn, FORMAT, __VA_ARGS__ ) ); \
   } \
  } while (0)

#define elog( FORMAT, ... ) \
  do { \
   if( (fc::logger::get()).is_enabled( fc::log_level::error ) ) { \
      (fc::logger::get()).log( FC_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
   } \
  } while (0)

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>


#define FC_FORMAT_ARG(r, unused, base) \
  BOOST_PP_STRINGIZE(base) ": ${" BOOST_PP_STRINGIZE( base ) "} "

#define FC_FORMAT_ARGS(r, unused, base) \
  BOOST_PP_LPAREN() BOOST_PP_STRINGIZE(base),fc::variant(base) BOOST_PP_RPAREN()

#define FC_FORMAT( SEQ )\
    BOOST_PP_SEQ_FOR_EACH( FC_FORMAT_ARG, v, SEQ ) 

#define FC_FORMAT_ARG_PARAMS( SEQ )\
    BOOST_PP_SEQ_FOR_EACH( FC_FORMAT_ARGS, v, SEQ ) 

#define idump( SEQ ) \
    ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  

