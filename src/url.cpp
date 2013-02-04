#include <fc/url.hpp>
#include <fc/value.hpp>
#include <fc/error_report.hpp>
#include <fc/lexical_cast.hpp>
#include <fc/value_cast.hpp>
#include <fc/sstream.hpp>

namespace fc {

//    url::url( const url& u );
#if 0
    url::url( url&& u )
    :proto(fc::move(u.proto)), 
     host(fc::move(u.host)), 
     user(fc::move(u.user)), 
     pass(fc::move(u.pass)), 
     path(fc::move(u.path)), 
     args(fc::move(u.args)), 
     port(u.port){}

    url& url::operator=(url&& c)
    { 
      fc::swap(*this,c);
      return *this;
    }

   // url::url& operator=(const url& c) {
   // }
#endif
    url::url( const string& s ) {
        from_string(s);
    }
    string url::to_string()const {
      fc::stringstream ss;
      ss<<proto<<"://";
      if( user ) {
        ss << *user;
        if( pass ) {
          ss<<":"<<*pass;
        }
        ss<<"@";
      }
      ss<<*host;
      if( port ) ss<<":"<<*port;
      if( path ) ss<<"/"<<*path;
      if( args ) ss<<"?"<<*args;
      return ss.str();
    }
    /**
     *  proto://user:pass@host:port/~/path?args
     *  proto://user:pass@host:port/absolute?args
     */
    url&   url::from_string( const string& s ) {
       fc::stringstream ss(s);
       fc::string skip,_user,_pass,_host,_port,_path,_args;
       fc::getline( ss, proto, ':' );
       fc::getline( ss, skip, '/' );
       fc::getline( ss, skip, '/' );

       if( s.find('@') != fc::string::npos ) {
         fc::string user_pass;
         fc::getline( ss, user_pass, '@' );
         fc::stringstream upss(user_pass);
         if( user_pass.find( ':' ) != fc::string::npos ) {
            fc::getline( upss, _user, ':' );
            fc::getline( upss, _pass, ':' );
            user = fc::move(_user);
            pass = fc::move(_pass);
         } else {
            user = fc::move(user_pass);
         }
       }
       fc::string host_port;
       fc::getline( ss, host_port, '/' );
       auto pos = host_port.find( ':' );
       if( pos != fc::string::npos ) {
          try {
          port = fc::lexical_cast<uint16_t>( host_port.substr( pos+1 ) );
          } catch ( ... ) {
            FC_THROW_REPORT( "Unable to parse port field in url", value().set( "url", s ) );
          }
          host = host_port.substr(0,pos);
       } else {
          host = fc::move(host_port);
       }
       fc::getline( ss, _path, '?' );
       fc::getline( ss, _args );
       path = fc::move(_path);
       if( _args.size() ) args = fc::move(_args);

       return *this;
    }

    bool url::operator==( const url& cmp )const {
      return cmp.proto == proto &&
             cmp.host  == host &&
             cmp.user  == user &&
             cmp.pass  == pass &&
             cmp.path  == path &&
             cmp.args  == args &&
             cmp.port  == port;
    }

} // namespace fc
