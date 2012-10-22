#pragma once
#include <fc/utility.hpp>
#include <fc/lexical_cast.hpp>

namespace fc {
  class string;

  class istream {
    public:
      virtual ~istream(){};

      virtual size_t readsome( char* buf, size_t len ) = 0;
      virtual istream& read( char* buf, size_t len ) = 0;

      template<typename T>
      friend istream& operator>>( istream& i, T& v ){ return i.read(v); }

      virtual bool eof()const = 0;

    protected:
      virtual istream& read( int64_t&    ) = 0;
      virtual istream& read( uint64_t&   ) = 0;
      virtual istream& read( int32_t&    ) = 0;
      virtual istream& read( uint32_t&   ) = 0;
      virtual istream& read( int16_t&    ) = 0;
      virtual istream& read( uint16_t&   ) = 0;
      virtual istream& read( int8_t&     ) = 0;
      virtual istream& read( uint8_t&    ) = 0;
      virtual istream& read( float&      ) = 0;
      virtual istream& read( double&     ) = 0;
      virtual istream& read( bool&       ) = 0;
      virtual istream& read( char&       ) = 0;
      virtual istream& read( fc::string& ) = 0;
  };

  class ostream {
    public:
      virtual ~ostream(){};

      virtual ostream& write( const char* buf, size_t len ) = 0;
      virtual void   close() = 0;
      virtual void   flush() = 0;

      template<typename T>
      friend ostream& operator<<( ostream& o, const T& v )         { return o.write(fc::lexical_cast<fc::string>(v)); }
      friend ostream& operator<<( ostream& o, char* v )            { return o.write(v); }
      friend ostream& operator<<( ostream& o, const char* v )      { return o.write(v); }
      friend ostream& operator<<( ostream& o, const fc::string& v ){ return o.write(v); }

    protected:
      virtual ostream& write( const fc::string& ) = 0;
  };
  class iostream : public virtual ostream, public virtual istream {};


  struct cout_t : virtual public ostream { 
      virtual ostream& write( const char* buf, size_t len );
      virtual void   close();
      virtual void   flush();

      virtual ostream& write( const fc::string& );
  };

  struct cerr_t : virtual public ostream { 
      virtual ostream& write( const char* buf, size_t len );
      virtual void   close();
      virtual void   flush();

      virtual ostream& write( const fc::string& );
  };

  struct cin_t : virtual public istream { 
      virtual size_t readsome( char* buf, size_t len );
      virtual istream& read( char* buf, size_t len );
      virtual bool eof()const;

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
  };
  fc::istream& getline( fc::istream&, fc::string&, char delim = '\n' );
  fc::cin_t&   getline( fc::cin_t&, fc::string&, char delim = '\n' );


  extern cout_t cout;
  extern cerr_t cerr;
  extern cin_t  cin;
}
