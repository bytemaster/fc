#pragma once
#include <fc/utility.hpp>
#include <fc/fwd.hpp>


/**
 *  There is debate about whether doing this is 'standard conforming', but 
 *  it works everywhere and enables the purpose of this library which is to 
 *  accelerate compiles while maintaining compatability.
 */
namespace std {
  template<class Char>
  struct char_traits;

  template<class T>
  class allocator;

  template<class Char, class Traits, class Allocator>
  class basic_string;

  typedef basic_string<char, char_traits<char>, allocator<char> > string;
}

namespace fc {
  /**
   *  Including <string> results in 4000 lines of code
   *  that must be included to build your header.  This
   *  class hides all of those details while maintaining
   *  compatability with std::string. Using fc::string
   *  instead of std::string can accelerate compile times
   *  10x.
   */
  class string {
    public:
      typedef char*       iterator;
      typedef const char* const_iterator;
      static const size_t npos = -1;

      string();
      string( const std::string& s );
      string( std::string&& s );
      string( const string& c );
      string( string&& c );
      string( const char* c );
      string( const char* c, int s );
      string( const_iterator b, const_iterator e );
      ~string();

      operator std::string&();
      operator const std::string&()const;

      iterator begin();
      iterator end();

      const_iterator begin()const;
      const_iterator end()const;

      char&       operator[](uint64_t idx);
      const char& operator[](uint64_t idx)const;

      string& operator =( const string& c );
      string& operator =( string&& c );

      void      reserve( uint64_t );
      uint64_t  size()const;
      uint64_t  find( char c, uint64_t pos = 0 )const;

      void    resize( uint64_t s );
      void    clear();

      const char* c_str()const;

      bool    operator == ( const char* s )const;
      bool    operator == ( const string& s )const;
      bool    operator != ( const string& s )const;

      friend bool operator < ( const string& a, const string& b );

      string& operator+=( const string& s );
      string& operator+=( char c );

      friend string operator + ( const string&, const string&  );
      friend string operator + ( const string&, char c );

      fc::string substr( int32_t start, int32_t len = 0x7fffffff )const;
    
    private:
       fc::fwd<std::string,32> my;
  };

} // namespace FC

