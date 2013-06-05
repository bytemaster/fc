#pragma once 
#include <fc/shared_ptr.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/iostream.hpp>

namespace fc {
  class path;
  class ofstream : virtual public ostream {
    public:
      enum mode { out, binary };
      ofstream();
      ofstream( const fc::path& file, int m = binary );
      ~ofstream();

      void open( const fc::path& file, int m = binary );
      size_t writesome( const char* buf, size_t len );
      void   put( char c );
      void   close();
      void   flush();

    private:
      class impl;
      fc::shared_ptr<impl> my;
  };

  class ifstream : virtual public istream {
    public:
      enum mode { in, binary };
      enum seekdir { beg, cur, end };

      ifstream();
      ifstream( const fc::path& file, int m );
      ~ifstream();

      void      open( const fc::path& file, int m );
      size_t    readsome( char* buf, size_t len );
      ifstream& read( char* buf, size_t len );
      ifstream& seekg( size_t p, seekdir d = beg );
      void      get( char& c ) { read( &c, 1 ); }
      void      close();
      bool      eof()const;
    private:
      class impl;
      fc::shared_ptr<impl> my;
  };
  
} // namespace fc 
