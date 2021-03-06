#pragma once
#include <fc/io/iostream.hpp>
#include <fc/fwd.hpp>

namespace fc {

  class stringstream : virtual public iostream {
    public:
      stringstream();
      stringstream( fc::string& s);
      stringstream( const fc::string& s);
      ~stringstream();

      fc::string str();
      void str(const fc::string& s);

      void clear();

      virtual bool     eof()const;
      virtual size_t   writesome( const char* buf, size_t len );
      virtual size_t   readsome( char* buf, size_t len );
      virtual void     close();
      virtual void     flush();
              char     peek();

    private:
      class impl;
      fwd<impl,368> my;
  };

}
