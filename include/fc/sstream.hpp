#pragma once
#include <fc/iostream.hpp>
#include <fc/fwd.hpp>

namespace fc {

  class stringstream : virtual public iostream {
    public:
      stringstream();
      stringstream( fc::string& s);
      stringstream( const fc::string& s);
      ~stringstream();

      fc::string str();

      virtual bool     eof()const;
      virtual ostream& write( const char* buf, size_t len );
      virtual size_t   readsome( char* buf, size_t len );
      virtual istream& read( char* buf, size_t len );
      virtual void     close();
      virtual void     flush();

    protected:
      virtual istream& read( int64_t&    );
      virtual istream& read( uint64_t&   );
      virtual istream& read( int32_t&    );
      virtual istream& read( uint32_t&   );
      virtual istream& read( int16_t&    );
      virtual istream& read( uint16_t&   );
      virtual istream& read( int8_t&     );
      virtual istream& read( uint8_t&    );
      virtual istream& read( float&      );
      virtual istream& read( double&     );
      virtual istream& read( bool&       );
      virtual istream& read( char&       );
      virtual istream& read( fc::string& );

      virtual ostream& write( const fc::string& );

    private:
      class impl;
      fwd<impl,368> my;
  };

}
