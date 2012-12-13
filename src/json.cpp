#include <fc/json.hpp>
#include <fc/hex.hpp>
#include <fc/exception.hpp>
#include <fc/sstream.hpp>
#include <fc/filesystem.hpp>
#include <fc/interprocess/file_mapping.hpp>
#include <fc/error_report.hpp>
#include <map>


namespace fc { namespace json {
  /**
   *  Placeholder for unparsed json data.
   */
  class string {
    public:
      template<typename T>
      string( T&& v ):json_data( fc::forward<T>(v) ){}
      string( const string& s ):json_data(s.json_data){}
      string( string&& s ):json_data(fc::move(s.json_data)){}
      string(){}

      string& operator=( const fc::string& s ) {
        json_data = fc::vector<char>(s.begin(),s.end());
        return *this;
      }
      template<typename T>
      string& operator=( T&& t ) {
        json_data = fc::forward<T>(t);
        return *this;
      }
      string& operator=( string&& s ) {
        json_data = fc::move(s.json_data);
        return *this;
      }
      string& operator=( const string& s )
      {
        json_data = s.json_data;
        return *this;
      }
      fc::vector<char> json_data;
  };

} }

typedef std::map<fc::string,fc::json::string> jmap;


  namespace errors {
    enum error_type {
      unknown_error       = 0x0001,  // Other errors not specified below
      warning             = 0x0002,  // Other warnigns not specified below
      sytnax_error        = 0x0004,  // fatal syntax errors unclosed brace
      sytnax_warning      = 0x0008,  // recoverable syntax error (missing, missing ':', unquoted string)
      missing_comma       = 0x0010,  // if the error was related to json syntax and not semantic
      string_to_int       = 0x0020,  // any time lexical cast from string to int is required
      double_to_int       = 0x0040,  // any time a double is received for an int
      int_overflow        = 0x0080,  // any time int value is greater than underlying type
      signed_to_unsigned  = 0x0100,  // any time a negative value is read for an unsigned field
      int_to_bool         = 0x0200,  // any time an int is read for a bool 
      string_to_bool      = 0x0400,  // any time a string is read for a bool
      bad_array_index     = 0x0800,  // atempt to read a vector field beyond end of sequence
      unexpected_key      = 0x1000,  // fields in object
      missing_key         = 0x2000,  // check required fields
      type_mismatch       = 0x4000,  // expected a fundamental, got object, expected object, got array, etc.
      type_conversion     = 0x8000,  // also set any time a conversion occurs
      all                 = 0xffff,
      none                = 0x0000
    };
  } // namespace errors

  /**
   *  Stores information about errors that occurred durring the parse.
   *
   *  By default extra fields are 'ignored' as warning
   *  Loss of presision errors are warning.
   *  String to Int conversion warnings
   *  Double to Int
   *  Int to bool
   *
   */
  struct parse_error {
    parse_error( int32_t ec, fc::string msg, char* s = 0, char* e = 0 )
    :message(fc::move(msg)),type(ec),start(s),end(e){}

    parse_error( parse_error&& m )
    :message(fc::move(m.message)),type(m.type),start(m.start),end(m.end){}

    fc::string message;
    int32_t     type;
    char*       start;
    char*       end;
  };

  /**
   *  Collects errors and manages how they are responded to.
   */
  class error_collector  {
    public:
      error_collector( error_collector&& e )
      :m_errors(fc::move(e.m_errors)){ 
        memcpy((char*)m_eclass,(char*)e.m_eclass, sizeof(m_eclass) );
      }
      /*
      error_collector( const error_collector&& e )
      :m_errors(e.m_errors){
        memcpy((char*)m_eclass,(char*)e.m_eclass, sizeof(m_eclass) );
      }
      */
      ~error_collector() throw() {
        try {
          m_errors.clear();
        }catch(...){}
      }

      enum error_defaults {
         default_report  = errors::all,
         default_recover = errors::all,
         default_throw   = errors::none,
         default_ignore  = ~(default_report|default_recover|default_throw)
      };

      error_collector(){
        m_eclass[report_error_t]  = default_report;
        m_eclass[recover_error_t] = default_recover;
        m_eclass[throw_error_t]   = default_throw;
        m_eclass[ignore_error_t]  = default_ignore;
      }

      inline bool report( int32_t e )const {
        return 0 != (m_eclass[report_error_t] & e);
      }
      inline bool recover( int32_t e )const {
        return 0 != (m_eclass[recover_error_t] & e);
      }
      inline bool ignore( int32_t e )const {
        return 0 != (m_eclass[ignore_error_t] & e);
      }

      void report_error( int32_t e ) {
        m_eclass[report_error_t] |= e;
        m_eclass[ignore_error_t] &= ~e;
      }
      void recover_error( int32_t e ) {
        m_eclass[recover_error_t] |= e;
        m_eclass[ignore_error_t] &= ~e;
      }
      void throw_error( int32_t e ) {
        m_eclass[throw_error_t]  |= e;
        m_eclass[ignore_error_t] &= ~e;
      }
      void ignore_error( int32_t e ) {
        m_eclass[ignore_error_t]  |= e;
        m_eclass[report_error_t]  &= ~m_eclass[ignore_error_t];
        m_eclass[recover_error_t] &= ~m_eclass[ignore_error_t];
        m_eclass[throw_error_t]   &= ~m_eclass[ignore_error_t];
      }

      void post_error( int32_t ec, fc::string msg, char* s = 0, char* e = 0 ) {
        m_errors.push_back( parse_error( ec, fc::move(msg), s, e ) ); 
        if( ec & m_eclass[throw_error_t] ) {
          throw fc::move(*this);
        }
      }
      const fc::vector<parse_error>& get_errors()const {
        return m_errors;
      }

    private:
      enum error_class {
        ignore_error_t,
        report_error_t,
        recover_error_t,
        throw_error_t,
        num_error_classes
      };
      uint32_t m_eclass[num_error_classes];
      fc::vector<parse_error>   m_errors;
  };
  fc::value to_value( char* start, char* end, error_collector& ec );










namespace fc { namespace json {
  fc::string escape_string( const fc::string& s ) {
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
    fc::string out; out.reserve(ecount);
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
  
  fc::string unescape_string( const fc::string& s ) {
    fc::string out; out.reserve(s.size());
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
            char c = fc::from_hex(*i);           
            ++i; if( i == out.end() ) { out += c;  return out; }
            c = c<<4 | fc::from_hex(*i);           
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
            char c = fc::from_hex(*i);           
            ++i; if( *i == '\0' ){ *out = c; ++out; *out = '\0'; return s; }
            c = c<<4 | fc::from_hex(*i);           
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
  } }// fc::json

/**
 *   Ignores leading white space.
 *   If it starts with [,", or { reads until matching ],", or }
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
 *   @return a pointer to the start of the value
 */
char* read_value( char* in, char* end, char*& oend ) {
   char* start = in;
   char* oin = in;
   // ignore leading whitespace
   while( (in < end) && ((*in == ' ') || (*in == '\t') || (*in == '\n') || (*in == '\r')) ) {
     ++in;
   }
   start = in;
   if( start == end ) {
    oend = end;
    return start;
   }

   bool found_dot = false;
   // check for literal vs object, array or string
   switch( *in ) {
     case ':':
     case ',':
     case '=':
        oend = in+1;
        return start;
     case '[':
     case '{':
     case '"':
       break;
     default: {  // literal
       // read until non-literal character
       // allow it to start with - 
       // allow only one '.' 
       while( in != end ) {
         switch( *in ) {
           case '[': case ']':
           case '{': case '}': 
           case ':': case '=':
           case ',': case '"': 
           case ' ': case '\t': case '\n': case '\r': { 
             oend = in;
             return start;
           }
           case '.':
             if( found_dot ) {
                oend = in;
                return start;
             }
             found_dot = true;
             break;
           case '-':
             if( in-start ){ oend = in; return start; }
         }
         ++in;
       }
       oend = in;
       return start;
     }
   } // end literal check

   int depth = 0;
   bool in_quote = false;
   bool in_escape = false;
   // read until closing ]} or " ignoring escaped "
   while( in != end ) {
     if( !in_quote ) {
       switch( *in ) {
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
       switch( *in ) {
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
     ++in;
     if( !depth ) { oend = in; return start; }
  }
  if( depth != 0 ) {
   // TODO: Throw Parse Error!
   elog("Parse Error!!  '%s' size: %d", fc::string( oin, end ).c_str(), (oin-start));
  }
  oend = in; return start;
}

struct temp_set {
  temp_set( char* v, char t )
  :old(*v),val(v) {  *val = t; }
  ~temp_set() { *val = old; }
  char  old;
  char* val;
};

/**
 * A,B,C
 * Warn on extra ',' or missing ','
 */
void  read_values( fc::value::array& a, char* in, char* end, error_collector& ec ) {
  char* ve = 0;
  char* v = read_value( in, end, ve );
  while( *v == ',' ) {
    wlog( "unexpected ','");
    v = read_value( ve, end, ve );
  }
  if( v == ve ) return; // no values

  { temp_set temp(ve,'\0'); a.fields.push_back( to_value( v, ve, ec ) ); }

  char* c;
  char* ce = 0;
  do { // expect comma + value | ''

    // expect comma or ''
    c = read_value( ve, end, ce );

    // '' means we are done, no errors
    if( c == ce ) return;

    if( *c != ',' ) // we got a value when expecting ','
    {
       wlog( "missing ," );
       temp_set temp(ce,'\0'); a.fields.push_back( to_value(c, ce, ec) );
       ve = ce;
       continue; // start back at start
    }
    
    // expect value
    v = read_value( ce, end, ve );
    while ( *v == ',' ) { // but got comma
      // expect value
      wlog( "extra comma at c->ce" );
      c = v; ce = ve;
      v = read_value( ve, end, ve );
    }
    if( v == ve ) {
      wlog( "trailing comma at c->ce" );
    } else { // got value
      temp_set temp(ve,'\0'); 
      a.fields.push_back( to_value( v, ve, ec) );
    }
  } while( ve < end );// expect comma + value | ''
}


/**
 *  Reads one level deep, does not recruse into sub objects.
 */
char* read_key_val( std::map<fc::string,fc::json::string>& obj, bool sc, char* in, char* end, error_collector& ec ) {
  char* name_end = 0;
  char* name = in;
  do {
    // read first char
    name = read_value( name, end, name_end );
    if( sc ) { // if we expect a ,
      if( *name != ',' ) { // but didn't get one
        wlog( "expected ',' but got %1%", name ); // warn and accept name
      } else { // we got the exepcted , read the expected name 
        name = read_value( name_end, end, name_end );
      }
    } else { // don't start with ','
      while( *name == ',' ) { //  while we don't have a name, keep looking
        wlog( "unexpected ',' " );
        name = read_value( name_end, end, name_end );
      }
    }
  } while( *name == ',' );

  
  // now we should have a name.
  if( name_end >= end -1 ) {
    temp_set ntemp(name_end,'\0');
    elog( "early end after reading name %1%", name );
    return name_end;
  }
  if( *name != '"' ) {
    temp_set ntemp(name_end,'\0');
    wlog( "unquoted name '%1%'", name );
  } else {
    temp_set ntemp(name_end,'\0');
    name = fc::json::inplace_unescape_string(name);
  }

  char* col_end = 0;
  char* col = read_value( name_end, end, col_end );

  char* val_end = 0;
  char* val = 0;
  
  bool sep_error = false;
  if( col_end-col == 1 ) {
    switch(  *col ) {
      case ':': break;
      case ';': 
      case '=': 
        wlog( "found %1% instead of ':'", *col );
        break;
      default:
        sep_error = true;
    }
  } else {
    sep_error = true;
  }

  if( sep_error ) {
    temp_set ntemp(name_end,'\0');
    temp_set vtemp(col_end,'\0');
    wlog( "expected ':' but got %1%", col );
    wlog( "assuming this is the value... " );
    val = col;
    val_end = col_end;
  }  else {
    if( name_end >= end -1 ) {
      temp_set ntemp(name_end,'\0');
      temp_set vtemp(col_end,'\0');
      elog( "early end after reading name '%1%' and col '%2%'", name, col );
      return name_end;
    }
    val = read_value( col_end, end, val_end );
    if( val == end ) {
      wlog( "no value specified" );
    }
  }
  temp_set ntemp(name_end,'\0');
  temp_set vtemp(val_end,'\0');
  //slog( "name: '%1%'", fc::string(name,name_end) );
  obj[name] = fc::vector<char>(val,val_end);
//  obj.fields.push_back( key_val( name, to_value( val, val_end, ec ) ) );
  return val_end;
}





/**
 *  Reads an optional ',' followed by key : value, returning the next input position
 *  @param sc - start with ','
 */
char* read_key_val( fc::value::object& obj, bool sc, char* in, char* end, error_collector& ec ) {
  char* name_end = 0;
  char* name = in;
  do {
    // read first char
    name = read_value( name, end, name_end );
    if( name == name_end )
      return name;
    if( sc ) { // if we expect a ,
      if( *name != ',' ) { // but didn't get one
        if( *name != '}' )
            wlog( "expected ',' or '}' but got '%s'", name ); // warn and accept name
      } else { // we got the exepcted , read the expected name 
        name = read_value( name_end, end, name_end );
      }
    } else { // don't start with ','
      while( *name == ',' ) { //  while we don't have a name, keep looking
        wlog( "unexpected ',' " );
        name = read_value( name_end, end, name_end );
      }
    }
  } while( *name == ',' );

  
  // now we should have a name.
  if( name_end >= end -1 ) {
    temp_set ntemp(name_end,'\0');
    elog( "early end after reading name '%s'", name );
    return name_end;
  }
  if( *name != '"' ) {
    temp_set ntemp(name_end,'\0');
    wlog( "unquoted name '%1%'", name );
  } else {
    temp_set ntemp(name_end,'\0');
    name = fc::json::inplace_unescape_string(name);
  }

  char* col_end = 0;
  char* col = read_value( name_end, end, col_end );

  char* val_end = 0;
  char* val = 0;
  
  bool sep_error = false;
  if( col_end-col == 1 ) {
    switch(  *col ) {
      case ':': break;
      case ';': 
      case '=': 
        wlog( "found %1% instead of ':'", *col );
        break;
      default:
        sep_error = true;
    }
  } else {
    sep_error = true;
  }

  if( sep_error ) {
    temp_set ntemp(name_end,'\0');
    temp_set vtemp(col_end,'\0');
    wlog( "expected ':' but got %1%", col );
    wlog( "assuming this is the value... " );
    val = col;
    val_end = col_end;
  }  else {
    if( name_end >= end -1 ) {
      temp_set ntemp(name_end,'\0');
      temp_set vtemp(col_end,'\0');
      elog( "early end after reading name '%1%' and col '%2%'", name, col );
      return name_end;
    }
    val = read_value( col_end, end, val_end );
    if( val == end ) {
      wlog( "no value specified" );
    }
  }
  temp_set ntemp(name_end,'\0');
  temp_set vtemp(val_end,'\0');
  //slog( "name: '%1%'", fc::string(name,name_end) );
  obj.fields.push_back( fc::value::key_val( name, to_value( val, val_end, ec ) ) );
  return val_end;
}

// first_key =::  '' | "name" : VALUE  *list_key
// list_key       '' | ',' "name" : VALUE
void read_key_vals( fc::value::object& obj, char* in, char* end, error_collector& ec ) {
  bool ex_c = false;
  char* kv_end = in;
  do {
    //slog( "%1% bytes to read", (end-kv_end) );
    kv_end = read_key_val( obj, ex_c, kv_end, end, ec );
    ex_c = true;
  } while( kv_end < end );
}
// first_key =::  '' | "name" : VALUE  *list_key
// list_key       '' | ',' "name" : VALUE
std::map<fc::string,fc::json::string> read_key_vals( char* in, char* end, error_collector& ec ) {
  std::map<fc::string,fc::json::string> m;
  bool ex_c = false;
  char* kv_end = in;
  do {
    //slog( "%1% bytes to read", (end-kv_end) );
    kv_end = read_key_val( m, ex_c, kv_end, end, ec );
    ex_c = true;
  } while( kv_end < end );
  return m;
}



/**
 *  @brief adaptor for to_value( char*, char*, error_collector& )
 */
fc::value to_value( fc::vector<char>&& v, error_collector& ec  ) {
  if( v.size() == 0 ) return fc::value();
  return to_value( &v.front(), &v.front() + v.size(), ec );
}

/**
 *  Returns a fc::value containing from the json string.
 *
 *  @param ec - determines how to respond to parse errors and logs
 *      any errors that occur while parsing the string.
 */
fc::value to_value( char* start, char* end, error_collector& ec ) {
  if( start == end ) return fc::value();

  char* ve = 0;
  char* s = read_value( start, end, ve );
  //slog( "'%1%'", fc::string(start,ve) );
  switch( s[0] ) {
    case '[': {
      fc::value::array a;
      read_values( a, s+1, ve -1, ec );
      return a;
    }
    case '{': {
      fc::value::object o;
      read_key_vals( o, s+1, ve -1, ec );
      fc::value v = fc::move(o);
      return v;
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      temp_set move_end(ve,'\0');
      for( char* n = s+1; n != ve; ++n ) {
        if( *n == '.' ) {
          return fc::lexical_cast<double>(s);
        }
      }
      return fc::lexical_cast<uint64_t>(s);
    }
    case '-': {
      temp_set move_end(ve,'\0');
      for( char* n = s+1; n != ve; ++n ) {
        if( *n == '.' ) {
          return fc::lexical_cast<double>(s);
        }
      }
      return fc::lexical_cast<int64_t>(s);
    }
    case '.': {
      temp_set move_end(ve,'\0');
      return fc::lexical_cast<int64_t>(s);
    }
    case '\"': {
      temp_set move_end(ve,'\0');
      return fc::json::inplace_unescape_string( s );  
    }
    case 'n': {
      temp_set move_end(ve,'\0');
      if( strcmp(s,"null" ) == 0) return fc::value();
    }
    case 't': {
      temp_set move_end(ve,'\0');
      if( strcmp(s,"true" ) == 0) return true;
    }
    case 'f': {
      temp_set move_end(ve,'\0');
      if( strcmp(s,"false" ) == 0) return false;
    }

    default:
      wlog( "return unable to parse... return as string '%s'", fc::string(s,ve).c_str() );
      return fc::value( fc::string( s, ve) );
  }
}
namespace fc { namespace json {

fc::string pretty_print( fc::vector<char>&& v, uint8_t indent ) {
  int level = 0;
  fc::stringstream ss;
  bool first = false;
  bool quote = false;
  bool escape = false;
  for( uint32_t i = 0; i < v.size(); ++i ) {
     switch( v[i] ) {
        case '\\':
          if( !escape ) {
            if( quote ) 
              escape = true;
          } else { escape = false; }
          ss<<v[i];
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
          ss<<v[i];
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
            if( v[i-1] != '[' && v[i-1] != '{' ) {
              ss<<'\n';
            }
            --level;
            if( !first ) {
              for( int i = 0; i < level*indent; ++i ) ss<<' ';
            }
            ss<<v[i];
            break;
          } else {
            escape = false;
            ss<<v[i];
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
          ss << v[i];
     }
  }
  return ss.str();
}
  

  template<typename Stream>
  struct value_visitor : value::const_visitor {
    value_visitor( Stream& s ):os(s){}
    Stream& os;
    virtual void operator()( const int8_t& v      ){ os << v; }
    virtual void operator()( const int16_t& v     ){ os << v; }
    virtual void operator()( const int32_t& v     ){ os << v; }
    virtual void operator()( const int64_t& v     ){ os << v; }
    virtual void operator()( const uint8_t& v     ){ os << v; }
    virtual void operator()( const uint16_t& v    ){ os << v; }
    virtual void operator()( const uint32_t& v    ){ os << v; }
    virtual void operator()( const uint64_t& v    ){ os << v; }
    virtual void operator()( const float& v       ){ os << v; }
    virtual void operator()( const double& v      ){ os << v; }
    virtual void operator()( const bool& v        ){ os << (v ? "true" : "false"); }
    virtual void operator()( const fc::string& v ){ os << '"' << fc::json::escape_string(v) <<'"'; }
    virtual void operator()( const fc::value::object& o ){
      os << '{';
        for( uint32_t i = 0; i < o.fields.size(); ++i ) {
          if( i ) os <<',';
          (*this)( o.fields[i].key );
          os<<':';
          o.fields[i].val.visit( value_visitor(*this) );
        }
      os << '}';
    }
    virtual void operator()( const value::array& o ){
      os << '[';
        for( uint32_t i = 0; i < o.fields.size(); ++i ) {
          if( i ) os <<',';
          o.fields[i].visit( value_visitor(*this) );
        }
      os << ']';
    }
    virtual void operator()( ){ os << "null"; }
  };

  template<typename Stream>
  void to_json( const fc::value& v, Stream& os ) {
    v.visit( value_visitor<Stream>(os) );
  }

   void write( ostream& out, const value& val ) {
     to_json( val, out );
   }

  fc::string to_string( const fc::value& v ) {
    fc::stringstream ss;
    to_json( v, ss );
    return ss.str();
  }

  value from_file( const fc::path& local_path ) {
    if( !exists(local_path) ) {
      FC_THROW_REPORT( "Source file ${filename} does not exist", value().set("filename",local_path.string()) );
    }
    if( is_directory( local_path ) ) {
      FC_THROW_REPORT( "Source path ${path} is a directory; a file was expected", 
                       value().set("path",local_path.string()) );
    }

    // memory map the file
    size_t       fsize = static_cast<size_t>(file_size(local_path));
    if( fsize == 0 ) { return value(); }
    file_mapping fmap( local_path.string().c_str(), read_only );


    mapped_region mr( fmap, fc::read_only, 0, fsize );

    const char* pos = reinterpret_cast<const char*>(mr.get_address());
    const char* end = pos + fsize;

    // TODO: implement a const version of to_value 
    fc::vector<char> tmp(pos,end);

    error_collector ec;
    return to_value(tmp.data(), tmp.data()+fsize,ec);
  }

  value from_string( const fc::string& s ) {
    return from_string( s.c_str(), s.c_str() + s.size() );
  }
  value from_string( fc::vector<char>&& v ) {
    error_collector ec;
    return to_value( v.data(), v.data() + v.size(), ec );
  }

  value from_string( const char* s, const char* e ) {
    return from_string( fc::vector<char>(s,e) );
  }

} }
