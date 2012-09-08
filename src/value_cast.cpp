#include <fc/value_cast.hpp>
#include <fc/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fc/error.hpp>


namespace fc {

  #define CAST_VISITOR_IMPL(X) \
   void reinterpret_value_visitor<X>::visit()const{\
     FC_THROW( bad_cast() );\
   } \
   void reinterpret_value_visitor<X>::visit( const char& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const uint8_t& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const uint16_t& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const uint32_t& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const uint64_t& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const int8_t& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const int16_t& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const int32_t& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const int64_t& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const double& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const float& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const bool& c )const{ _s = c; } \
   void reinterpret_value_visitor<X>::visit( const string& c )const{\
    _s = boost::lexical_cast<X>( reinterpret_cast<const std::string&>(c) ); \
   } \
   void reinterpret_value_visitor<X>::visit( const char* member, int idx, int size, \
                                                                const cref& v)const{\
     FC_THROW( bad_cast() );\
   }\
   void reinterpret_value_visitor<X>::visit( int idx, int size, const cref& v)const{\
     FC_THROW( bad_cast() );\
   }

  CAST_VISITOR_IMPL(int64_t);
  CAST_VISITOR_IMPL(int32_t);
  CAST_VISITOR_IMPL(int16_t);
  CAST_VISITOR_IMPL(int8_t);
  CAST_VISITOR_IMPL(uint64_t);
  CAST_VISITOR_IMPL(uint32_t);
  CAST_VISITOR_IMPL(uint16_t);
  CAST_VISITOR_IMPL(uint8_t);
  CAST_VISITOR_IMPL(double);
  CAST_VISITOR_IMPL(float);
  CAST_VISITOR_IMPL(bool);

  #undef CAST_VISITOR_IMPL

   void reinterpret_value_visitor<string>::visit()const{
     FC_THROW( bad_cast() );
   } 
   void reinterpret_value_visitor<string>::visit( const char& c )const{
    slog("" );
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c);
   } 
   void reinterpret_value_visitor<string>::visit( const uint8_t& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const uint16_t& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const uint32_t& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const uint64_t& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const int8_t& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const int16_t& c )const{
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const int32_t& c )const{
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const int64_t& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const double& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const float& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }
   void reinterpret_value_visitor<string>::visit( const bool& c )const{ 
     reinterpret_cast<std::string&>(_s) = boost::lexical_cast<std::string>(c); }

   void reinterpret_value_visitor<string>::visit( const string& c )const{
      slog( "" );
      _s = c;
   }
   void reinterpret_value_visitor<string>::visit( const char* member, int idx, int size, 
                                                                const cref& v)const{
     elog( "%s", member );
     FC_THROW( bad_cast() );
   }
   void reinterpret_value_visitor<string>::visit( int idx, int size, const cref& v)const{
     elog( "%d of %d", idx, size );
     FC_THROW( bad_cast() );
   }


}
