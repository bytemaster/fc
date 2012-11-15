#pragma once 
#include <fc/fwd.hpp>
#include <fc/iostream.hpp>

namespace fc {
  class path;
  class ofstream : virtual public ostream {
    public:
      enum mode { out, binary };
      ofstream();
      ofstream( const fc::path& file, int m = binary );
      ~ofstream();

      void open( const fc::path& file, int m = binary );
      ofstream& write( const char* buf, size_t len );
      void   put( char c );
      void   close();
      void   flush();

    private:
      class impl;
      fwd<impl,896> my;
  };

  class ifstream : virtual public istream {
    public:
      enum mode { in, binary };
      ifstream();
      ifstream( const fc::path& file, int m );
      ~ifstream();

      void open( const fc::path& file, int m );
      ifstream& read( char* buf, size_t len );
      void   close();
      void   flush();

      bool    eof()const;

    private:
      class impl;
      fwd<impl,904> my;
  };
  
} // namespace fc 
