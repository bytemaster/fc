#include <fc/json.hpp>
#include <fc/reflect_value.hpp>
#include <fc/stream.hpp>
#include <fc/value.hpp>
#include <fc/fwd_impl.hpp>

#include <boost/lexical_cast.hpp>

// TODO: replace sstream with light/fast compiling version
#include <sstream>

namespace fc { namespace json {


    struct range {
        range( const char* s, const char* e )
        :start(s),end(e){ }

        operator bool()const  { return start < end; }
        char operator*()const { return *start; }

        range& operator++()    { ++start; return *this; }
        range& operator++(int) { ++start; return *this; }

        operator string() { return string(start,end); }

        const char* start;
        const char* end;
    };


  
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
      virtual void visit( const fc::string& c )const{ 
        out << '"'<< escape_string(c)<<'"'; 
      }
      virtual void array_size( int size )const {
          if( size == 0 ) { out <<"[]"; }
      }
      virtual void object_size( int size )const {
          if( size == 0 ) { out <<"{}"; }
      }
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
  /*
  class visitor : public fc::abstract_visitor {
    public:
      virtual void visit()const{}
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
  */
   uint8_t from_hex( char c ) {
     if( c >= '0' && c <= '9' )
       return c - '0';
     if( c >= 'a' && c <= 'f' )
         return c - 'a' + 10;
     if( c >= 'A' && c <= 'F' )
         return c - 'A' + 10;
     return c;
   }

   value to_value( char* start, char* end/*, error_collector& ec*/ );

  /**
   *  Any unescaped quotes are dropped. 
   *  Because unescaped strings are always shorter, we can simply reuse
   *  the memory of s.
   *  
   *  @param s a null terminated string that contains one or more escape chars
   */
  char* inplace_unescape_string( char* s ) {
    while( *s == '"' ) ++s;
    char* out = s;

    for( auto i = s; *i != '\0'; ++i ) {
      if( *i != '\\' ) {
        if( *i != '"' ) {
            *out = *i;
            ++out; 
        }
      }
      else {
        ++i; 
        if( *i == '\0' ) { *out = '\0'; return s; }
        switch( *i ) {
          case 't' : *out = '\t'; ++out; break;
          case 'n' : *out = '\n'; ++out; break;
          case 'r' : *out = '\r'; ++out; break;
          case '\\': *out = '\\'; ++out; break;
          case '"' : *out = '"'; ++out; break;
          case 'x' : { 
            ++i; if( *i == '\0' ){ *out = '\0'; return s; }
            char c = from_hex(*i);           
            ++i; if( *i == '\0' ){ *out = c; ++out; *out = '\0'; return s; }
            c = c<<4 | from_hex(*i);           
            *out = c;
            ++out;
            break;
          }
          default:
            *out = '\\';
            ++out; 
            *out = *i;
            ++out;
        }
      }
    }
    *out = '\0';
    return s;
  }


   string to_string( const cref& o ) {
      std::stringstream ss;
      {
      fc::ostream os(ss);
      o._reflector.visit( o._obj, fc::json::const_visitor(os) );
      }
      return ss.str().c_str();
   }
  

   string pretty_print( const char* v, size_t s, uint8_t indent ) {
     int level = 0;
     const char* e= v + s;
     std::stringstream ss;
     bool first = false;
     bool quote = false;
     bool escape = false;
     while( v < e ) {
        switch( *v ) {
           case '\\':
             if( !escape ) {
               if( quote ) 
                 escape = true;
             } else { escape = false; }
             ss<<*v;
             break;
           case ':':
             if( !quote ) {
               ss<<": ";
             } else {
               ss<<':';
             }
             break;
           case '"':
             if( first ) {
                ss<<'\n';
                for( int i = 0; i < level*indent; ++i ) ss<<' ';
                first = false;
             }
             if( !escape ) {
               quote = !quote;
             } 
             escape = false;
             ss<<'"';
             break;
           case '{':
           case '[':
             ss<<*v;
             if( !quote ) {
               ++level;
               first = true;
             }else {
               escape = false;
             }
             break;
           case '}':
           case ']':
             if( !quote ) {
               if( *(v-1) != '[' && *(v-1) != '{' ) {
                 ss<<'\n';
               }
               --level;
               if( !first ) {
                 for( int i = 0; i < level*indent; ++i ) ss<<' ';
               }
               ss<<*v;
               break;
             } else {
               escape = false;
               ss<<*v;
             }
           case ',':
             if( !quote ) {
               ss<<',';
               first = true;
             } else {
               escape = false;
               ss<<',';
             }
             break;
           default:
             if( first ) {
                ss<<'\n';
                for( int i = 0; i < level*indent; ++i ) ss<<' ';
                first = false;
             }
             ss << *v;
        }
     }
     return ss.str().c_str();
   }

   string to_pretty_string( const cref& o, uint8_t indent ) {
      auto s =  to_string(o);
      return pretty_print( s.c_str(), s.size(), indent );
   }



    /**
     *   Ignores leading white space.
     *   If it starts with [,", or  reads until matching ],", or 
     *   If it starts with something else it reads until [{",}]: or whitespace only
     *        allowing a starting - or a single .
     *
     *   @note internal json syntax errors are not caught, only bracket errors 
     *         are caught by this method.  This makes it easy for error recovery
     *         when values are read recursivley.
     *   
     *   @param in    start of input
     *   @param end   end of input
     *   @param oend  output parameter to the end of the value
     *   
     *   @return the range of inputs for the value
     */
    range read_value( const range& in ) {
         range start = in;
         // ignore leading whitespace
         bool done = false;
         while( !done && start ) {
            switch( *start ) {
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                case ',':
                  ++start;
                default:
                  done = true;
            }
         }
         if( !start ) return start;
         range out = start;
    
         bool found_dot = false;
         // check for literal vs object, array or string
         switch( *start ) {
           case ':':
           case ',':
           case '=':
              out.end = start.start + 1;
              return out;
           case '[':
           case '{':
           case '"':
             break;
           default: {  // literal
             // read until non-literal character
             // allow it to start with - 
             // allow only one '.' 
             while( start ) {
               switch( *start ) {
                 case '[': case ']':
                 case '{': case '}': 
                 case ':': case '=':
                 case ',': case '"': 
                 case ' ': case '\t': case '\n': case '\r': { 
                   out.end = start.start;                                             
                   return out;
                 }
                 case '.':
                   if( found_dot ) {
                      out.end = start.start;                                             
                      return out;
                   }
                   found_dot = true;
                   break;
                 case '-':
                   if( out.start-start.start ){ 
                      out.end = start.start;                                             
                      return out;
                   }
               }
               ++start;
             }
             out.end = start.start;                                             
             return out;
           }
         } // end literal check
    
         int depth = 0;
         bool in_quote = false;
         bool in_escape = false;
         // read until closing ] or " ignoring escaped "
     while( start ) {
       if( !in_quote ) {
         switch( *start) {
           case '[':
           case '{': ++depth;         break;
           case ']':
           case '}': --depth;         break;
           case '"': 
             ++depth;
             in_quote = true; 
             break;
           default: // do nothing;
             break;
         }
       } else { // in quote
         switch( *start ) {
           case '"': if( !in_escape ) {
             --depth;
             in_quote = false;
             break;
           }
           case '\\': 
             in_escape = !in_escape;
             break;
           default:
             in_escape = false;
         }
       }
       ++start;
       if( !depth ) { return range( out.start, start.start ); }
    }
    if( depth != 0 ) {
     // TODO: Throw Parse Error!
     elog("Parse Error!!");
    }
    return range( out.start, start.start ); 
  }




   void write( ostream& out, const cref& val ) {
      val._reflector.visit( val._obj, fc::json::const_visitor(out) );
   }

   void read( istream& in, ref& val ) {

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

    range skip_separator( range r, char c ) {
      while( r ) {
        switch( *r ) {
          case ' ': case '\n': case '\t': case '\r': 
               ++r;
              continue;
          default:
              if( *r == c ) { ++r; }
              else {
     //       wlog( "Expected ',' but found '%c'", *r );
              }
              return r;
        }
      }
    }

    /**
     * [A,B,C]
     */
    value  read_array( const range& r ) {
       BOOST_ASSERT( *r == '[' );
       BOOST_ASSERT( *(r.end-1) == ']' );
       value out;
       range cur_range = read_value( r );

       while( cur_range ) {
         out.push_back( *from_string( cur_range.start, cur_range.end ) );
         cur_range = read_value( range( cur_range.end, r.end) );
       }
       return out;
    }

    /**
     *  @pre *input == {
     *  @pre *input.end-1 == }
     */
    value read_object( const range& in ) {
       BOOST_ASSERT( *in == '{' );
       BOOST_ASSERT( *(in.end-1) == '}' );
       value v;
       range key = read_value( ++range(in) );
       range rest(key.end,in.end-1);
       while( rest ) {
          range val      = skip_separator( range(key.end,in.end), ':' );
          val            = read_value( val );
          v[string(key)] = *from_string( val.start, val.end );
          range key      = read_value( rest );
          rest.start     = key.end;
       }
       return v;
    }

    value_fwd from_string( const string& s ) {
       return from_string( s.c_str(), s.c_str() + s.size() );
    }
    
    value_fwd from_string( const char* start, const char* end ) {
      if( start == end ) return value();
    
      range s = read_value( range(start,end) );
      switch( s.start[0] ) {
        case '[': 
          return read_array( s );
        case '{': 
          return read_object( s );
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
          for( const char* n = s.start+1; n != s.end; ++n ) {
            if( *n == '.' ) {
              return boost::lexical_cast<double>(std::string(s.start,s.end));
            }
          }
          return boost::lexical_cast<uint64_t>(std::string(s.start,s.end));
        }
        case '-': {
          for( const char* n = s.start+1; n != s.end; ++n ) {
            if( *n == '.' ) {
              return (value)boost::lexical_cast<double>(std::string(s.start,s.end));
            }
          }
          return (value)boost::lexical_cast<int64_t>(std::string(s.start,s.end));
        }
        case '.': {
          return (value)boost::lexical_cast<int64_t>(std::string(s.start,s.end));
        }
        case '\"': {
          return (value)unescape_string( string(s.start,s.end) );  
        }
        case 'n': {
          if( strncmp(s.start,"null",4 ) ) return value();
        }
        case 't': {
          if( strncmp(s.start,"true",4 ) ) return true;
        }
        case 'f': {
          if( strncmp(s.start,"false",5 ) ) return false;
        }
        default:
          wlog( "return unable to parse... return as string" );
          return value( string( s.start, s.end) );
      }
    }

} } // fc::json
