#pragma once
#include <fc/string.hpp>

namespace fc {
  
  namespace detail {
    template<typename T, typename R>
    struct lexical_cast { };

    double to_double( const fc::string& s );
    inline double to_double( double d ) { return d; }

    int64_t to_int64( const fc::string& s );
    inline  int64_t to_int64( double d ) { return static_cast<int64_t>(d); }
    inline  int64_t to_int64( int64_t d ) { return d; }

    uint64_t        to_uint64( const fc::string& s );
    inline uint64_t to_uint64( double d ) { return static_cast<uint64_t>(d); }
    inline uint64_t to_uint64( uint64_t d ) { return d; }

    fc::string to_string( double d   );
    fc::string to_string( size_t d   );
    fc::string to_string( uint64_t d );
    fc::string to_string( uint32_t d  );
    fc::string to_string( uint16_t d  );
    fc::string to_string( uint8_t d  );
    fc::string to_string( int64_t d  );
    fc::string to_string( int32_t d  );
    fc::string to_string( int16_t d  );
    fc::string to_string( int8_t d  );
    fc::string to_string( char d  );
    inline fc::string to_string( const char* d  ) { return d; }
    inline fc::string to_string( fc::string s ) { return s; }

    template<typename R> 
    struct lexical_cast<double, R> {
       static double cast( const R& v ) { return to_double( v ); }
    };

    template<typename R> 
    struct lexical_cast<fc::string, R> {
       static fc::string cast( const R& v ) { return to_string( v ); }
    };
    template<typename R> 
    struct lexical_cast<std::string, R> {
       static std::string cast( const R& v ) { return to_string( v ); }
    };

    template<typename R> 
    struct lexical_cast<uint64_t, R> {
       static uint64_t cast( const R& v ) { return to_uint64( v ); }
    };

    template<typename R> 
    struct lexical_cast<int64_t, R> { static int64_t cast( const R& v ) { return static_cast<int64_t>(to_int64( v )); } };

    template<typename R> 
    struct lexical_cast<int32_t, R> { static int32_t cast( const R& v ) { return static_cast<int32_t>(to_int64( v )); } };

    template<typename R> 
    struct lexical_cast<int16_t, R> { static int16_t cast( const R& v ) { return static_cast<int16_t>(to_int64( v )); } };

    template<typename R> 
    struct lexical_cast<int8_t, R> { static int8_t cast( const R& v ) { return static_cast<int8_t>(to_int64( v )); } };


    template<typename R> 
    struct lexical_cast<uint32_t, R> { static uint32_t cast( const R& v ) { return static_cast<uint32_t>(to_uint64( v )); } };

    template<typename R> 
    struct lexical_cast<uint16_t, R> { static uint16_t cast( const R& v ) { return static_cast<uint16_t>(to_uint64( v )); } };

    template<typename R> 
    struct lexical_cast<uint8_t, R> { static uint8_t cast( const R& v ) { return static_cast<uint8_t>(to_uint64( v )); } };

    template<typename R> 
    struct lexical_cast<bool, R> { static bool cast( const R& v ) { return v != 0; } };

    template<> 
    struct lexical_cast<char, fc::string> { static char cast( const fc::string& v ) 
    { return v[0]; } };// TODO: check string len

    template<> 
    struct lexical_cast<bool, fc::string> { static bool cast( const fc::string& v ) { return v == "true"; } };

    template<> 
    struct lexical_cast<fc::string,bool> { static fc::string cast( const bool& v ) { return v ? "true" : "false";} };

    template<typename R> 
    struct lexical_cast<float, R> { static float cast( const R& v ) { return static_cast<float>(to_double( v )); } };
  }


  template<typename T, typename R>
  T lexical_cast( const R& v ) {
    return detail::lexical_cast<T,R>::cast(v);
  }
}
