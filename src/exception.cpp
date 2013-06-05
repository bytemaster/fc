#include <fc/exception/exception.hpp>
#include <boost/exception/all.hpp>
#include <fc/io/sstream.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/json.hpp>

namespace fc
{
   namespace detail
   {
      enum exception_code 
      {
          unspecified_exception_code  = 0, ///< for exceptions we threw that don't have an assigned code
          unhandled_exception_code    = 1, ///< for unhandled 3rd party exceptions
          timeout_exception_code      = 2, ///< timeout exceptions
          file_not_found_exception_code = 3,
          parse_error_exception_code   = 4,
          invalid_arg_exception_code   = 5,
          key_not_found_exception_code = 6,
          bad_cast_exception_code      = 7,
          out_of_range_exception_code  = 8,
          canceled_exception_code      = 9,
          assert_exception_code        = 10,
          eof_exception_code           = 11,
          std_exception_code           = 12,
      };

     void  to_variant( detail::exception_code e, variant& v )
     {
        switch( e )
        {
          case unhandled_exception_code:
             v = "unhandled";
             break;
          case timeout_exception_code:
             v = "timeout";
             break;
          case key_not_found_exception_code:
             v = "invalid_key";
             break;
          case bad_cast_exception_code:
             v = "bad_cast";
             break;
          case file_not_found_exception_code:
             v = "file_not_found";
             break;
          case parse_error_exception_code:
             v = "parse_error";
             break;
          case invalid_arg_exception_code:
             v = "invalid_arg";
             break;
          case out_of_range_exception_code:
             v = "out_of_range";
             break;
          case canceled_exception_code:
             v = "canceled";
             break;
          case assert_exception_code:
             v = "assert";
             break;
          case std_exception_code:
             v = "std";
             break;
          case eof_exception_code:
             v = "eof";
             break;
          case unspecified_exception_code:
          default:
             v = "unspecified";
             break;
             
        }
     }
     void  from_variant( const variant& e, detail::exception_code& ll )
     {
        string v = e.as_string();
        if( v == "unspecified" )         ll = unspecified_exception_code;
        else if( v == "unhandled" )      ll = unhandled_exception_code;
        else if( v == "timeout" )        ll = timeout_exception_code;
        else if( v == "key_not_found" )  ll = key_not_found_exception_code;
        else if( v == "bad_cast" )       ll = bad_cast_exception_code;
        else if( v == "file_not_found" ) ll = file_not_found_exception_code;
        else if( v == "parse_error" )    ll = parse_error_exception_code;
        else if( v == "invalid_arg" )    ll = invalid_arg_exception_code;
        else if( v == "out_of_range" )   ll = out_of_range_exception_code;
        else if( v == "canceled" )       ll = canceled_exception_code;
        else if( v == "assert" )         ll = assert_exception_code;
        else if( v == "std" )            ll = std_exception_code;
        else if( v == "eof" )            ll = eof_exception_code;
        else  FC_THROW_EXCEPTION( bad_cast_exception, 
                                       "Invalid Error Report _code  '${code}'",
                                       ("code", v) );
     }

      class exception_impl
      {
         public:
            exception_code  _ecode;
            log_messages    _elog;
            variant         _props;
      };
   }

   std_exception::std_exception( log_message&& m, std::exception_ptr e, const char* w ) 
   :unhandled_exception( fc::move(m), e ) 
   { 
      my->_ecode = detail::std_exception_code; 
      _what  = w;
   } 
   std_exception::std_exception( const exception& e )
   :unhandled_exception(e)
   {
      from_variant( my->_props );
   }

   std_exception::std_exception( log_messages m )  
   :unhandled_exception(fc::move(m) )
   {
     my->_ecode = detail::std_exception_code; 
   } 

   void std_exception::from_variant( const variant& v )
   {
      _what = v.get_object()["what"].as_string();
   }
   void std_exception::to_variant( variant& v )
   {
      v = variant_object( "what", _what );
   }


   unhandled_exception::unhandled_exception( log_message&& m, std::exception_ptr e ) 
   :exception( fc::move(m) ) 
   { 
      my->_ecode = detail::unhandled_exception_code; 
      _inner = e;
   } 
   unhandled_exception::unhandled_exception( const exception& r )
   :exception(r)
   {
   }
   unhandled_exception::unhandled_exception( log_messages m )  
   :exception() 
   { my->_elog = fc::move(m); 
     my->_ecode = detail::unhandled_exception_code; 
   } 
   std::exception_ptr unhandled_exception::get_inner_exception()const { return _inner; }
   NO_RETURN void     unhandled_exception::dynamic_rethrow_exception()const
   {
      if( !(_inner == std::exception_ptr()) ) std::rethrow_exception( _inner );
      else { fc::exception::dynamic_rethrow_exception(); }
   }
   std::shared_ptr<exception> unhandled_exception::dynamic_copy_exception()const
   {
      auto e = std::make_shared<unhandled_exception>( *this );
      e->_inner = _inner;
      return e;
   }

#define FC_EXCEPTION_IMPL( TYPE ) \
   TYPE::TYPE( log_message&& m ) \
   :exception( fc::move(m) ) { my->_ecode = detail::TYPE ##_code; } \
   TYPE::TYPE(){ my->_ecode = detail::TYPE ##_code; } \
   TYPE::TYPE(const TYPE& t):exception(t){} \
   TYPE::TYPE( log_messages m )  \
   :exception() { my->_elog = fc::move(m); my->_ecode = detail::TYPE ##_code; } 

  FC_EXCEPTION_IMPL(timeout_exception)
  FC_EXCEPTION_IMPL(file_not_found_exception)
  FC_EXCEPTION_IMPL(parse_error_exception)
  FC_EXCEPTION_IMPL(invalid_arg_exception)
  FC_EXCEPTION_IMPL(key_not_found_exception)
  FC_EXCEPTION_IMPL(bad_cast_exception)
  FC_EXCEPTION_IMPL(out_of_range_exception)
  FC_EXCEPTION_IMPL(canceled_exception)
  FC_EXCEPTION_IMPL(assert_exception)
  FC_EXCEPTION_IMPL(eof_exception)




   exception::exception()
   :my( new detail::exception_impl() )
   {
      my->_ecode = detail::unspecified_exception_code;
   }

   exception::exception( log_message&& msg)
   :my( new detail::exception_impl() )
   {
      my->_ecode = detail::unspecified_exception_code;
      my->_elog.push_back( fc::move( msg ) );
   }
   exception::exception( const exception& c )
   :my( new detail::exception_impl(*c.my) )
   {
   }
   exception::exception( exception&& c )
   :my( fc::move(c.my) ){}

   exception::~exception(){}


   void to_variant( const exception& e, variant& v )
   {
      v = mutable_variant_object( "stack", e.my->_elog )
                                ( "type", e.my->_ecode) 
                                ( "props", e.my->_props );
   }
   void          from_variant( const variant& v, exception& ll )
   {
      auto obj = v.get_object();
      ll.my->_elog  = obj["stack"].as<log_messages>();
      ll.my->_ecode = obj["type"].as<detail::exception_code>();
      ll.my->_props = obj["props"];
   }

   const log_messages&   exception::get_log()const { return my->_elog; }
   void                 exception::append_log( log_message m )
   {
      my->_elog.push_back( fc::move(m) );
   }
   
   /**
    *   Generates a detailed string including file, line, method,
    *   and other information that is generally only useful for
    *   developers.
    */
   string exception::to_detail_string( log_level ll  )const
   {
      fc::stringstream ss;
      ss << variant(my->_ecode).as_string() <<"\n";
      for( auto itr = my->_elog.begin(); itr != my->_elog.end();  )
      {
         ss << itr->get_message() <<"\n"; //fc::format_string( itr->get_format(), itr->get_data() ) <<"\n";
         ss << "    " << json::to_string( itr->get_data() )<<"\n";
         ss << "    " << itr->get_context().to_string();
         ++itr;
         if( itr != my->_elog.end() ) ss<<"\n";
      }
      return ss.str();
   }

   /**
    *   Generates a user-friendly error report.
    */
   string exception::to_string( log_level ll   )const
   {
      fc::stringstream ss;
      ss << what() << "(" << variant(my->_ecode).as_string() <<")\n";
      for( auto itr = my->_elog.begin(); itr != my->_elog.end(); ++itr )
      {
         ss << fc::format_string( itr->get_format(), itr->get_data() ) <<"\n";
   //      ss << "    " << itr->get_context().to_string() <<"\n";
      }
      return ss.str();
   }

   /**
    * Rethrows the exception restoring the proper type based upon
    * the error code.  This is used to propagate exception types
    * across conversions to/from JSON
    */
   NO_RETURN void  exception::dynamic_rethrow_exception()const
   {
      switch( my->_ecode )
      {
        case detail::unhandled_exception_code:
           throw unhandled_exception( my->_elog );
        case detail::timeout_exception_code:
           throw timeout_exception( my->_elog );
        case detail::key_not_found_exception_code:
           throw key_not_found_exception( my->_elog );
        case detail::bad_cast_exception_code:
           throw bad_cast_exception( my->_elog );
        case detail::parse_error_exception_code:
           throw parse_error_exception( my->_elog );
        case detail::canceled_exception_code:
           throw canceled_exception( my->_elog);
        case detail::assert_exception_code:
           throw assert_exception( my->_elog );
        case detail::file_not_found_exception_code:
           throw file_not_found_exception( my->_elog );
        case detail::invalid_arg_exception_code:
           throw invalid_arg_exception( my->_elog );
        case detail::out_of_range_exception_code:
           throw out_of_range_exception( my->_elog );
        case detail::eof_exception_code:
           throw eof_exception( my->_elog );
        case detail::std_exception_code:
           throw std_exception( *this );
        case detail::unspecified_exception_code:
        default:
           throw fc::exception(*this);
      }
   }
   exception_ptr exception::dynamic_copy_exception()const 
   {
      switch( my->_ecode )
      {
        case detail::unhandled_exception_code:
           return std::make_shared<unhandled_exception>( my->_elog );
        case detail::timeout_exception_code:
           return std::make_shared<timeout_exception>( my->_elog );
        case detail::key_not_found_exception_code:
           return std::make_shared<key_not_found_exception>( my->_elog );
        case detail::bad_cast_exception_code:
           return std::make_shared<bad_cast_exception>( my->_elog );
        case detail::parse_error_exception_code:
           return std::make_shared<parse_error_exception>( my->_elog );
        case detail::canceled_exception_code:
           return std::make_shared<canceled_exception>( my->_elog);
        case detail::assert_exception_code:
           return std::make_shared<assert_exception>( my->_elog );
        case detail::file_not_found_exception_code:
           return std::make_shared<file_not_found_exception>( my->_elog );
        case detail::invalid_arg_exception_code:
           return std::make_shared<invalid_arg_exception>( my->_elog );
        case detail::out_of_range_exception_code:
           return std::make_shared<out_of_range_exception>( my->_elog );
        case detail::eof_exception_code:
           return std::make_shared<eof_exception>( my->_elog );
        case detail::std_exception_code:
           return std::make_shared<std_exception>( *this );
        case detail::unspecified_exception_code:
        default:
           return std::make_shared<exception>(*this);
      }
   }
   fc::string except_str()
   {
       return boost::current_exception_diagnostic_information(); 
   }
   void throw_bad_enum_cast( int64_t i, const char* e )
   {
      FC_THROW_EXCEPTION( bad_cast_exception, "invalid index '${key}' in enum '${enum}'", ("key",i)("enum",e) );
   }
   void throw_bad_enum_cast( const char* k, const char* e )
   {
      FC_THROW_EXCEPTION( bad_cast_exception, "invalid name '${key}' in enum '${enum}'", ("key",k)("enum",e) );
   }

} // fc
