#pragma once
/**
 *  @file exception.hpp
 *  @brief Defines exception's used by fc 
 */
#include <fc/log/log_message.hpp>
#include <fc/optional.hpp>
#include <exception>

namespace fc
{
   namespace detail { class exception_impl; }

   /**
    *  @brief Used to generate a useful error report when an exception is thrown.
    *  @ingroup serializable
    *
    *  At each level in the stack where the exception is caught and rethrown a
    *  new log_message is added to the exception.
    *
    *  exception's are designed to be serialized to a variant and
    *  deserialized from an variant.  
    *
    *  @see FC_THROW_EXCEPTION
    *  @see FC_RETHROW_EXCEPTION
    *  @see FC_RETHROW_EXCEPTIONS
    */
   class exception 
   {
      public:
         exception();
         exception( log_message&& );
         exception( const exception& e );
         exception( exception&& e );
         ~exception();

         virtual const char*  what()const throw() { return "exception"; }

         /**
          *   @return a reference to log messages that have
          *   been added to this log.
          */
         const log_messages&  get_log()const;
         void                 append_log( log_message m );
         
         /**
          *   Generates a detailed string including file, line, method,
          *   and other information that is generally only useful for
          *   developers.
          */
         string to_detail_string( log_level ll = log_level::all )const;

         /**
          *   Generates a user-friendly error report.
          */
         string to_string( log_level ll = log_level::info  )const;

         /**
          *  Throw this exception as its most derived type. 
          *
          *  @note does not return.
          */
         virtual NO_RETURN void     dynamic_rethrow_exception()const;

         /**
          *  This is equivalent to:
          *  @code
          *   try { throwAsDynamic_exception(); } 
          *   catch( ... ) { return std::current_exception(); }
          *  @endcode
          */
         virtual std::shared_ptr<exception> dynamic_copy_exception()const;

      protected:
         friend void to_variant( const exception& e, variant& v );
         friend void from_variant( const variant& e, exception& ll );
         virtual void from_variant( const variant&  ){}
         virtual void to_variant( variant&  ){}

         std::unique_ptr<detail::exception_impl> my;
   };
   void to_variant( const exception& e, variant& v );
   void from_variant( const variant& e, exception& ll );
   typedef std::shared_ptr<exception> exception_ptr;

   typedef optional<exception> oexception;



   /**
    *  @brief re-thrown whenever an unhandled exception is caught.
    *  @ingroup serializable
    *  Any exceptions thrown by 3rd party libraries that are not
    *  caught get wrapped in an unhandled_exception exception.  
    *
    *  The original exception is captured as a std::exception_ptr 
    *  which may be rethrown.  The std::exception_ptr does not
    *  propgate across process boundaries. 
    */
   class unhandled_exception : public exception 
   { 
      public: 
       unhandled_exception( log_message&& m, std::exception_ptr e = std::current_exception() ); 
       unhandled_exception( log_messages ); 
       unhandled_exception( const exception&  ); 
       virtual const char*  what()const throw() { return "Unhandled _exception"; } 
       std::exception_ptr get_inner_exception()const;

       virtual NO_RETURN void     dynamic_rethrow_exception()const;
       virtual std::shared_ptr<exception> dynamic_copy_exception()const;
      private:
       std::exception_ptr _inner;
   };

   template<typename T>
   fc::exception_ptr copy_exception( T&& e )
   {
#if defined(_MSC_VER) && (_MSC_VER < 1700)
     return std::make_shared<unhandled_exception>( log_message(), std::copy_exception(fc::forward<T>(e)) );
#else
     return std::make_shared<unhandled_exception>( log_message(), std::make_exception_ptr(fc::forward<T>(e)) );
#endif
   }

   /**
    *  @brief wraps unhanlded std::exception's 
    *  @ingroup serializable
    *
    *  This exception allows the 'what' field of unhandled std::exceptions
    *  to be propagated across process boundaries.
    */
   class std_exception : public unhandled_exception
   { 
      public: 
       std_exception( log_message&& m, std::exception_ptr e, const char* w ); 
       std_exception( log_messages ); 
       std_exception( const exception& c); 
       virtual const char*  what()const throw() { return _what.c_str(); }

      protected:
       void from_variant( const variant& v );
       void to_variant( variant& v );
      private:
       string  _what;
   };


#define FC_DECLARE_EXCEPTION( TYPE, WHAT ) \
   class TYPE : public exception \
   { \
      public: \
       TYPE( log_message&& m ); \
       TYPE( log_messages ); \
       TYPE( const TYPE& c ); \
       TYPE(); \
       virtual const char*  what()const throw() { return WHAT; } \
   };

  FC_DECLARE_EXCEPTION( timeout_exception, "Timeout" );
  FC_DECLARE_EXCEPTION( file_not_found_exception, "File Not Found" );
  /**
   * @brief report's parse errors
   */
  FC_DECLARE_EXCEPTION( parse_error_exception, "Parse Error" );
  FC_DECLARE_EXCEPTION( invalid_arg_exception, "Invalid Argument" );
  /**
   * @brief reports when a key, guid, or other item is not found.
   */
  FC_DECLARE_EXCEPTION( key_not_found_exception, "Key Not Found" );
  FC_DECLARE_EXCEPTION( bad_cast_exception, "Bad Cast" );
  FC_DECLARE_EXCEPTION( out_of_range_exception, "Out of Range" );

  /** @brief if an operation is unsupported or not valid this may be thrown */
  FC_DECLARE_EXCEPTION( invalidOperation_exception, "Invalid Operation" );

  /**
   *  @brief used to report a canceled Operation
   */
  FC_DECLARE_EXCEPTION( canceled_exception, "Canceled" );
  /**
   *  @brief used inplace of assert() to report violations of pre conditions.
   */
  FC_DECLARE_EXCEPTION( assert_exception, "Assert Exception" );
  FC_DECLARE_EXCEPTION( eof_exception, "End Of File" );

  fc::string except_str();


} // namespace fc

/**
 *@brief: Workaround for varying preprocessing behavior between MSVC and gcc
 */
#define FC_EXPAND_MACRO( x ) x
/**
 *  @brief Checks a condition and throws an assert_exception if the test is FALSE
 */
#define FC_ASSERT( TEST, ... ) \
FC_EXPAND_MACRO( \
do { if( !(TEST) ) { FC_THROW_EXCEPTION( assert_exception, #TEST ": "  __VA_ARGS__ ); } } while(0); \
)

#define FC_THROW( FORMAT, ... ) \
   do { \
   throw fc::exception( FC_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) );  \
   } while(0)

#define FC_EXCEPTION( EXCEPTION_TYPE, FORMAT, ... ) \
    fc::EXCEPTION_TYPE( FC_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) )  
/**
 *  @def FC_THROW_EXCEPTION( EXCEPTION, FORMAT, ... )
 *  @param EXCEPTION a class in the Phoenix::Athena::API namespace that inherits
 *  @param format - a const char* string with "${keys}"
 */
#define FC_THROW_EXCEPTION( EXCEPTION, FORMAT, ... ) \
   do { \
   throw fc::EXCEPTION( FC_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
   } while(0)


/**
 *  @def FC_RETHROW_EXCEPTION(ER,LOG_LEVEL,FORMAT,...)
 *  @brief Appends a log_message to the exception ER and rethrows it.
 */
#define FC_RETHROW_EXCEPTION( ER, LOG_LEVEL, FORMAT, ... ) \
  do { \
     er.append_log( FC_LOG_MESSAGE( LOG_LEVEL, FORMAT, __VA_ARGS__ ) ); \
     throw;\
  } while(false)

/**
 *  @def FC_RETHROW_EXCEPTIONS(LOG_LEVEL,FORMAT,...)
 *  @brief  Catchs all exception's, std::exceptions, and ... and rethrows them after
 *   appending the provided log message.
 */
#define FC_RETHROW_EXCEPTIONS( LOG_LEVEL, FORMAT, ... ) \
   catch( fc::exception& er ) { \
      FC_RETHROW_EXCEPTION( er, LOG_LEVEL, FORMAT, __VA_ARGS__ ); \
   } catch( const std::exception& e ) {  \
      throw  fc::std_exception( \
                FC_LOG_MESSAGE( LOG_LEVEL, FORMAT,__VA_ARGS__), \
                std::current_exception(), \
                e.what() ) ; \
   } catch( ... ) {  \
      throw fc::unhandled_exception( \
                FC_LOG_MESSAGE( LOG_LEVEL, FORMAT,__VA_ARGS__), \
                std::current_exception() ); \
   }

