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

      virtual bool eof()const = 0;
  };

  class ostream {
    public:
      virtual ~ostream(){};

      virtual ostream& write( const char* buf, size_t len ) = 0;
      virtual void   close(){}
      virtual void   flush(){}
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
      ~cin_t();
      virtual size_t readsome( char* buf, size_t len );
      virtual istream& read( char* buf, size_t len );
      virtual bool eof()const;
  };
  fc::istream& getline( fc::istream&, fc::string&, char delim = '\n' );


  extern cout_t cout;
  extern cerr_t cerr;
  extern cin_t  cin;


  template<typename T>
  ostream& operator<<( ostream& o, const T& v ) {
      auto str = fc::lexical_cast<fc::string>(v); 
      o.write( str.c_str(), static_cast<size_t>(str.size()) );
      return o;
  }
  ostream& operator<<( ostream& o, const char* v );

  template<typename T>
  ostream& operator<<( ostream& o, const fc::string& str ) {
      o.write( str.c_str(), static_cast<size_t>(str.size()) );
      return o;
  }
  template<typename T>
  istream& operator>>( istream& o, T& v ) {
      fc::string str;
      getline( o, str, ' ' );
      v = fc::lexical_cast<T>(str); 
      return o;
  }
}
