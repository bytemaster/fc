#include <fc/lexical_cast.hpp>
#include <boost/lexical_cast.hpp>
namespace fc {

  namespace detail {
    double   to_double( const fc::string& s ) {
      return boost::lexical_cast<double>(s.c_str());
    }
    int64_t  to_int64( const fc::string& s ) {
      return boost::lexical_cast<int64_t>(s.c_str());
    }
    uint64_t to_uint64( const fc::string& s ) {
      return boost::lexical_cast<uint64_t>(s.c_str());
    }
    fc::string to_string( double d   ){ return boost::lexical_cast<std::string>(d); }
    #ifdef __APPLE__
    fc::string to_string( size_t d   ){ return boost::lexical_cast<std::string>(d); }
    #endif
    fc::string to_string( uint64_t d ){ return boost::lexical_cast<std::string>(d); }
    fc::string to_string( uint32_t d  ){ return boost::lexical_cast<std::string>(d); }
    fc::string to_string( uint16_t d  ){ return boost::lexical_cast<std::string>(d); }
    fc::string to_string( uint8_t d  ){ return boost::lexical_cast<std::string>(d); }
    fc::string to_string( int64_t d  ){ return boost::lexical_cast<std::string>(d); }
    fc::string to_string( int32_t d  ){ return boost::lexical_cast<std::string>(d); }
    fc::string to_string( int16_t d  ){ return boost::lexical_cast<std::string>(d); }
    fc::string to_string( int8_t d  ){ return boost::lexical_cast<std::string>(d); }
    fc::string to_string( char d  ){ return boost::lexical_cast<std::string>(d); }
  }
}
