#ifndef _FC_STRING_HPP_
#define _FC_STRING_HPP_
#include <stdint.h>

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

      string();
      string( const string& c );
      string( string&& c );
      string( const char* c );
      string( const_iterator b, const_iterator e );
      ~string();

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

      string& operator+=( const string& s );
      string& operator+=( char c );

      friend string operator + ( const string&, const string&  );
      friend string operator + ( const string&, char c );
    
    private:
       void* my;
  };

} // namespace FC

#endif // _FC_STRING_HPP_
