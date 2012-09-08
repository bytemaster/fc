#include <fc/json.hpp>
#include <fc/reflect.hpp>
#include <fc/stream.hpp>

// TODO: replace sstream with light/fast compiling version
#include <sstream>

namespace fc { namespace json {
  
  class const_visitor : public fc::abstract_const_visitor {
    public:
      fc::ostream&  out;
      const_visitor( fc::ostream& o ):out(o){}

      virtual void visit()const{}
      virtual void visit( const char& c )const{ out << '"' << c << '"'; }
      virtual void visit( const uint8_t& c )const{ out << int(c); }
      virtual void visit( const uint16_t& c )const{ out << c; }
      virtual void visit( const uint32_t& c )const{ out << c; }
      virtual void visit( const uint64_t& c )const{ out << c; }
      virtual void visit( const int8_t& c )const{ out << int(c); }
      virtual void visit( const int16_t& c )const{ out << c;}
      virtual void visit( const int32_t& c )const{ out << c;}
      virtual void visit( const int64_t& c )const{ out << c;}
      virtual void visit( const double& c )const{ out << c;}
      virtual void visit( const float& c )const{ out << c;}
      virtual void visit( const bool& c )const{ out << (c?"true":"false"); }
      virtual void visit( const fc::string& c )const{ out << '"'<<c.c_str()<<'"'; }
      virtual void visit( const char* member, int idx, int size, const cref& v)const{
          if( !idx ) out <<"{"; 
          out<<'"'<<member<<"\":";
          v._reflector.visit( v._obj, *this );
          if( idx != size-1 ) {
            out <<',';
          } else {
            out << '}';
          }
      }
      virtual void visit( int idx, int size, const cref& v)const{
          if( !idx ) out << '[';
          v._reflector.visit( v._obj, *this );
          out << ((idx < (size -1) ) ? ',' : ']');
      }
  };

  class visitor : public fc::abstract_visitor {
    public:
      virtual void visit(){}
      virtual void visit( char& c )const{}
      virtual void visit( uint8_t& c )const{}
      virtual void visit( uint16_t& c )const{}
      virtual void visit( uint32_t& c )const{}
      virtual void visit( uint64_t& c )const{}
      virtual void visit( int8_t& c )const{}
      virtual void visit( int16_t& c )const{}
      virtual void visit( int32_t& c )const{}
      virtual void visit( int64_t& c )const{}
      virtual void visit( double& c )const{}
      virtual void visit( float& c )const{}
      virtual void visit( bool& c )const{}
      virtual void visit( fc::string& c )const{}
      virtual void visit( const char* member, int idx, int size, const ref& v)const{}
      virtual void visit( int idx, int size, const ref& v)const{}
  };

   string to_string( const cref& o ) {
      std::stringstream ss;
      {
      fc::ostream os(ss);
      o._reflector.visit( o._obj, fc::json::const_visitor(os) );
      }
      return ss.str().c_str();
   }
   void  from_string( const string& s, const ref& o ) {
    
   }

   void write( ostream& out, const cref& val ) {
      val._reflector.visit( val._obj, fc::json::const_visitor(out) );
   }

   uint8_t from_hex( char c ) {
     if( c >= '0' && c <= '9' )
       return c - '0';
     if( c >= 'a' && c <= 'f' )
         return c - 'a' + 10;
     if( c >= 'A' && c <= 'F' )
         return c - 'A' + 10;
     return c;
   }

   string escape_string( const string& s ) {
      // calculate escape string size.
      uint32_t ecount = 0;
      for( auto i = s.begin(); i != s.end(); ++i ) {
        if( ' '<= *i && *i <= '~' &&  *i !='\\' && *i != '"' ) {
          ecount+=1;
        } else {
          switch( *i ) {
            case '\t' : 
            case '\n' : 
            case '\r' : 
            case '\\' : 
            case '"' : 
              ecount += 2; break;
            default: 
              ecount += 4;
          }
        }
      }
      // unless the size changed, just return it.
      if( ecount == s.size() ) { return s; }
      
      // reserve the bytes
      string out; out.reserve(ecount);
      
      // print it out.
      for( auto i = s.begin(); i != s.end(); ++i ) {
        if( ' '<= *i && *i <= '~' &&  *i !='\\' && *i != '"' ) {
          out += *i;
        } else {
          out += '\\';
          switch( *i ) {
            case '\t' : out += 't'; break;
            case '\n' : out += 'n'; break;
            case '\r' : out += 'r'; break;
            case '\\' : out += '\\'; break;
            case '"' :  out += '"'; break;
            default: 
              out += "x";
              const char* const hexdig = "0123456789abcdef";
              out += hexdig[*i >> 4];
              out += hexdig[*i & 0xF];
          }
        }
      }
      return out;
   }
   string unescape_string( const string& s ) {
    string out; out.reserve(s.size());
    for( auto i = s.begin(); i != s.end(); ++i ) {
      if( *i != '\\' ) {
        if( *i != '"' ) out += *i;
      }
      else {
        ++i; 
        if( i == out.end() ) return out;
        switch( *i ) {
          case 't' : out += '\t'; break;
          case 'n' : out += '\n'; break;
          case 'r' : out += '\r'; break;
          case '\\' : out += '\\'; break;
          case '"' :  out += '"'; break;
          case 'x' : { 
            ++i; if( i == out.end() ) return out;
            char c = from_hex(*i);           
            ++i; if( i == out.end() ) { out += c;  return out; }
            c = c<<4 | from_hex(*i);           
            out += c;
            break;
          }
          default:
            out += '\\';
            out += *i;
        }
      }
    }
    return out;
   }




} }
