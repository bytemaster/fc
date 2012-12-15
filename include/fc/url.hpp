#pragma once
#include <fc/string.hpp>
#include <fc/optional.hpp>
#include <stdint.h>

namespace fc {

  typedef fc::optional<fc::string> ostring;
  
  struct url {
    url(){}
    url( const string& url );
    /*
    url( const url& u );

    url( url&& u );
    url& operator=(url&& c);
    url& operator=(const url& c);
    */

    operator string()const { return to_string(); }
    string to_string()const;
    url&   from_string( const string& s );

    bool operator==( const url& cmp )const;

    string                    proto; // file, ssh, tcp, http, ssl, etc...
    ostring                   host;
    ostring                   user;
    ostring                   pass;
    ostring                   path;
    ostring                   args;
    fc::optional<uint16_t>    port;
  };

} // namespace fc

#include <fc/reflect.hpp>
FC_REFLECT( fc::url,  (proto)(host)(user)(pass)(path)(args)(port) )

