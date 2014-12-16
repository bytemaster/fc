#pragma once

// This file is an internal header,
// it is not meant to be included except internally from json.cpp in fc

#include <fc/io/json.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/iostream.hpp>
#include <fc/io/buffered_iostream.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/sstream.hpp>
#include <fc/log/logger.hpp>
#include <fc/string.hpp>
//#include <utfcpp/utf8.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/filesystem/fstream.hpp>

namespace fc { namespace json_relaxed
{
   template<typename T, bool strict>
   variant variant_from_stream( T& in );

   template<typename T>
   fc::string tokenFromStream( T& in )
   {
      fc::stringstream token;
      try
      {
         char c = in.peek();

         while( true )
         {
            switch( c = in.peek() )
            {
               case '\\':
                  token << parseEscape( in );
                  break;
               case '\t':
               case ' ':
               case ',':
               case ':':
               case '\0':
               case '\n':
               case '\x04':
                  in.get();
                  return token.str();
               case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
               case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
               case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
               case 'y': case 'z':
               case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
               case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
               case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
               case 'Y': case 'Z':
               case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
               case '8': case '9':
               case '_': case '-': case '.': case '+': case '/':
                  token << c;
                  in.get();
                  break;
               default:
                  return token.str();
            }
         }
         return token.str();
      }
      catch( const fc::eof_exception& eof )
      {
         return token.str();
      }
      catch (const std::ios_base::failure&)
      {
         return token.str();
      }

      FC_RETHROW_EXCEPTIONS( warn, "while parsing token '${token}'",
                                          ("token", token.str() ) );
   }

   template<typename T, bool strict, bool allow_escape>
   fc::string quoteStringFromStream( T& in )
   {
       fc::stringstream token;
       try
       {
           char q = in.get();
           switch( q )
           {
               case '\'':
                   if( strict )
                       FC_THROW_EXCEPTION( parse_error_exception, "expected: '\"' at beginning of string, got '\''" );
                   // falls through
               case '"':
                   break;
               default:
                   if( strict )
                       FC_THROW_EXCEPTION( parse_error_exception, "expected: '\"' at beginning of string" );
                   else
                       FC_THROW_EXCEPTION( parse_error_exception, "expected: '\"' | '\\\'' at beginning of string" );
           }
           if( in.peek() == q )
           {
               in.get();
               if( in.peek() != q )
                   return fc::string();

               // triple quote processing
               if( strict )
                   FC_THROW_EXCEPTION( parse_error_exception, "triple quote unsupported in strict mode" );
               else
               {
                   in.get();
                   
                   while( true )
                   {
                       char c = in.peek();
                       if( c == q )
                       {
                           in.get();
                           char c2 = in.peek();
                           if( c2 == q )
                           {
                               char c3 = in.peek();
                               if( c3 == q )
                               {
                                   in.get();
                                   return token.str();
                               }
                               token << q << q;
                               continue;
                           }
                           token << q;
                           continue;
                       }
                       else if( c == '\x04' )
                           FC_THROW_EXCEPTION( parse_error_exception, "unexpected EOF in string '${token}'",
                                      ("token", token.str() ) );
                       else if( allow_escape && (c == '\\') )
                           token << parseEscape( in );
                       else
                       {
                           in.get();
                           token << c;
                       }
                   }
               }
           }
           
           while( true )
           {
               char c = in.peek();

               if( c == q )
               {
                   in.get();
                   return token.str();
               }
               else if( c == '\x04' )
                   FC_THROW_EXCEPTION( parse_error_exception, "unexpected EOF in string '${token}'",
                              ("token", token.str() ) );
               else if( allow_escape && (c == '\\') )
                   token << parseEscape( in );
               else if( (c == '\r') | (c == '\n') )
                   FC_THROW_EXCEPTION( parse_error_exception, "unexpected EOL in string '${token}'",
                              ("token", token.str() ) );
               else
               {
                   in.get();
                   token << c;
               }
           }
           
       } FC_RETHROW_EXCEPTIONS( warn, "while parsing token '${token}'",
                                          ("token", token.str() ) );
   }

   template<typename T, bool strict>
   fc::string stringFromStream( T& in )
   {
      try
      {
         char c = in.peek(), c2;

         switch( c )
         {
             case '\'':
                 if( strict )
                     FC_THROW_EXCEPTION( parse_error_exception, "expected: '\"' at beginning of string, got '\''" );
                 // falls through
             case '"':
                 return quoteStringFromStream<T, strict, true>( in );
             case 'r':
                 if( strict )
                     FC_THROW_EXCEPTION( parse_error_exception, "raw strings not supported in strict mode" );
             case 'R':
                 in.get();
                 c2 = in.peek();
                 switch( c2 )
                 {
                     case '"':
                     case '\'':
                         if( strict )
                             FC_THROW_EXCEPTION( parse_error_exception, "raw strings not supported in strict mode" );
                         return quoteStringFromStream<T, strict, false>( in );
                     default:
                         if( strict )
                             FC_THROW_EXCEPTION( parse_error_exception, "unquoted strings not supported in strict mode" );
                         return c+tokenFromStream( in );
                 }
                 break;
             case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
             case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
             case 'q':           case 's': case 't': case 'u': case 'v': case 'w': case 'x':
             case 'y': case 'z':
             case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
             case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
             case 'Q':           case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
             case 'Y': case 'Z':
             case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
             case '8': case '9':
             case '_': case '-': case '.': case '+': case '/':
                 if( strict )
                     FC_THROW_EXCEPTION( parse_error_exception, "unquoted strings not supported in strict mode" );
                 return tokenFromStream( in );
             default:
                 FC_THROW_EXCEPTION( parse_error_exception, "expected: string" );
         }

       } FC_RETHROW_EXCEPTIONS( warn, "while parsing string" );
   }
   
   struct CharValueTable
   {
       public:
           CharValueTable()
           {
               for( size_t i=0; i<0x100; i++ )
                   c2v[i] = 0xFF;
               c2v['0'] = 0;
               c2v['1'] = 1;
               c2v['2'] = 2;
               c2v['3'] = 3;
               c2v['4'] = 4;
               c2v['5'] = 5;
               c2v['6'] = 6;
               c2v['7'] = 7;
               c2v['8'] = 8;
               c2v['9'] = 9;
               c2v['a'] = c2v['A'] = 10;
               c2v['b'] = c2v['B'] = 11;
               c2v['c'] = c2v['C'] = 12;
               c2v['d'] = c2v['D'] = 13;
               c2v['e'] = c2v['E'] = 14;
               c2v['f'] = c2v['F'] = 15;
               c2v['g'] = c2v['G'] = 16;
               c2v['h'] = c2v['H'] = 17;
               c2v['i'] = c2v['I'] = 18;
               c2v['j'] = c2v['J'] = 19;
               c2v['k'] = c2v['K'] = 20;
               c2v['l'] = c2v['L'] = 21;
               c2v['m'] = c2v['M'] = 22;
               c2v['n'] = c2v['N'] = 23;
               c2v['o'] = c2v['O'] = 24;
               c2v['p'] = c2v['P'] = 25;
               c2v['q'] = c2v['Q'] = 26;
               c2v['r'] = c2v['R'] = 27;
               c2v['s'] = c2v['S'] = 28;
               c2v['t'] = c2v['T'] = 29;
               c2v['u'] = c2v['U'] = 30;
               c2v['v'] = c2v['V'] = 31;
               c2v['w'] = c2v['W'] = 32;
               c2v['x'] = c2v['X'] = 33;
               c2v['y'] = c2v['Y'] = 34;
               c2v['z'] = c2v['Z'] = 35;
               return;
           }
           
           uint8_t operator[]( char index ) const { return c2v[index & 0xFF]; }
       
       uint8_t c2v[0x100];
   };
   
   template<uint8_t base>
   fc::variant parseInt( const fc::string& token, size_t start )
   {
       static const CharValueTable ctbl;
       static const uint64_t INT64_MAX_PLUS_ONE = static_cast<uint64_t>(INT64_MAX) + 1;
       
       size_t i = start, n = token.length();
       if( i >= n )
           FC_THROW_EXCEPTION( parse_error_exception, "zero-length integer" );
       
       uint64_t val = 0;
       uint64_t maxb4mul = UINT64_MAX / base;
       
       while(true)
       {
           char c = token[i];
           uint8_t vc = ctbl[c];
           if( vc == 0xFF )
               FC_THROW_EXCEPTION( parse_error_exception, "illegal character {c} in integer of base {b}", ("c", c)("b", base) );
           if( val > maxb4mul )
               FC_THROW_EXCEPTION( parse_error_exception, "integer literal overflow" );
           val *= base;
           uint64_t newval = val + vc;
           if( newval < val )
               FC_THROW_EXCEPTION( parse_error_exception, "integer literal overflow" );
           val = newval;
           i++;
           if( i >= n )
               break;
       }
       if( token[0] == '-' )
       {
           if( val > INT64_MAX_PLUS_ONE )
               FC_THROW_EXCEPTION( parse_error_exception, "negative integer literal overflow" );
           // special cased to avoid trying to compute -INT64_MIN which is probably undefined or something
           if( val == INT64_MAX_PLUS_ONE )
               return fc::variant( INT64_MIN );
           return fc::variant( -static_cast<int64_t>(val) );
       }
       return fc::variant( val );
   }

   template<bool strict, uint8_t base>
   fc::variant maybeParseInt( const fc::string& token, size_t start )
   {
       try
       {
           return parseInt<base>( token, start );
       }
       catch( const parse_error_exception &e )
       {
           if( strict )
               throw( e );
           else
               return fc::variant( token );
       }
   }

   template<bool strict>
   fc::variant parseNumberOrStr( const fc::string& token )
   {
       
       size_t i = 0, n = token.length();
       if( n == 0 )
           FC_THROW_EXCEPTION( parse_error_exception, "expected: non-empty token, got: empty token" );
       switch( token[0] )
       {
           case '+':
               if( strict )
                   FC_THROW_EXCEPTION( parse_error_exception, "unary + not supported in strict mode" );
               i++;
               break;
           case '-':
               i++;
               break;
           default:
               break;
       }
       char c = token[i++];
       switch( c )
       {
           case '0':
               if( i >= n )
                   return fc::variant( uint64_t( 0 ) );
               switch( token[i] )
               {
                   case 'b':
                   case 'B':
                       if( strict )
                           FC_THROW_EXCEPTION( parse_error_exception, "binary numeric literals not supported in strict mode" );
                       i++;
                       if( i >= n )
                           FC_THROW_EXCEPTION( parse_error_exception, "empty binary numeric literal" );
                       return maybeParseInt<strict, 2>( token, i+1 );
                   case 'o':
                   case 'O':
                       if( strict )
                           FC_THROW_EXCEPTION( parse_error_exception, "octal numeric literals not supported in strict mode" );
                       return maybeParseInt<strict, 8>( token, i+1 );
                   case 'x':
                   case 'X':
                       if( strict )
                           FC_THROW_EXCEPTION( parse_error_exception, "hex numeric literals not supported in strict mode" );
                       return maybeParseInt<strict, 16>( token, i+1 );
                   case '.':
                   case 'e':
                   case 'E':
                       break;
                   default:
                       // since this is a lookahead, other cases will be treated later
                       if( strict )
                           FC_THROW_EXCEPTION( parse_error_exception, "expected '.'|'e'|'E' parsing number, got '{c}'",
                               ( "c", c ) );
               }
               break;
           case '1': case '2': case '3': case '4':
           case '5': case '6': case '7': case '8': case '9':
               break;
           case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
           case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
           case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
           case 'y': case 'z':
           case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
           case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
           case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
           case 'Y': case 'Z':
           case '_': case '-': case '.': case '+': case '/':
               if( strict )
                   FC_THROW_EXCEPTION( parse_error_exception, "illegal character '{c}' parsing number", ( "c", c ) );
               return fc::variant( token );
           default:
               FC_THROW_EXCEPTION( parse_error_exception, "illegal character '{c}' in token", ( "c", c ) );
       }
       size_t start = i-1;

       bool dot_ok = true;

       // int frac? exp?
       while(true)
       {
           if( i >= n )
               return parseInt<10>( token, start );
           char c = token[i++];
           switch( c )
           {
               case '0': case '1': case '2': case '3': case '4':
               case '5': case '6': case '7': case '8': case '9':
                   break;
               case '.':
                   if( dot_ok )
                   {
                       dot_ok = false;
                       if( i == n )
                       {
                           if( strict )
                               FC_THROW_EXCEPTION( parse_error_exception, "number cannot end with '.' in strict mode" );
                           return fc::variant( fc::to_double(token.c_str()) );
                       }

                       c = token[i+1];
                       switch( c )
                       {
                           case '0': case '1': case '2': case '3': case '4':
                           case '5': case '6': case '7': case '8': case '9':
                               break;
                           case 'e':
                           case 'E':
                               if( strict )
                                   FC_THROW_EXCEPTION( parse_error_exception, "expected digit after '.'" );
                               break;
                           case 'a': case 'b': case 'c': case 'd':           case 'f': case 'g': case 'h':
                           case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
                           case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                           case 'y': case 'z':
                           case 'A': case 'B': case 'C': case 'D':           case 'F': case 'G': case 'H':
                           case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
                           case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                           case 'Y': case 'Z':
                           case '_': case '-': case '.': case '+': case '/':
                               if( strict )
                                   FC_THROW_EXCEPTION( parse_error_exception, "expected digit after '.'" );
                               return fc::variant( token );
                           default:
                               FC_THROW_EXCEPTION( parse_error_exception, "illegal character '{c}' in token", ( "c", c ) );
                       }
                   }
                   else
                   {
                       if( strict )
                           FC_THROW_EXCEPTION( parse_error_exception, "illegal multiple . in number" );
                       return fc::variant( token );
                   }
                   break;
               case 'e':
               case 'E':
                   if( i == n )
                   {
                       if( strict )
                           FC_THROW_EXCEPTION( parse_error_exception, "expected exponent after 'e'|'E' parsing number" );
                       return fc::variant( token );
                   }
                   c = token[i++];
                   switch( c )
                   {
                       case '+': case '-':
                           if( i == n )
                           {
                               if( strict )
                                   FC_THROW_EXCEPTION( parse_error_exception, "expected exponent" );
                               return fc::variant( token );
                           }
                           break;
                       case '0': case '1': case '2': case '3': case '4':
                       case '5': case '6': case '7': case '8': case '9':
                           break;
                       case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
                       case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
                       case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                       case 'y': case 'z':
                       case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
                       case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
                       case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                       case 'Y': case 'Z':
                       case '_':           case '.':           case '/':
                           if( strict )
                               FC_THROW_EXCEPTION( parse_error_exception, "illegal character '{c}' in number", ( "c", c ) );
                           return fc::variant( token );
                       default:
                           FC_THROW_EXCEPTION( parse_error_exception, "illegal character '{c}' in token", ( "c", c ) );
                   }
                   while( true )
                   {
                       if( i == n )
                           break;
                       c = token[i++];
                       switch( c )
                       {
                           case '0': case '1': case '2': case '3': case '4':
                           case '5': case '6': case '7': case '8': case '9':
                               break;
                           case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
                           case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
                           case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                           case 'y': case 'z':
                           case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
                           case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
                           case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                           case 'Y': case 'Z':
                           case '_': case '-': case '.': case '+': case '/':
                               if( strict )
                                   FC_THROW_EXCEPTION( parse_error_exception, "illegal character '{c}' in number", ( "c", c ) );
                               return fc::variant( token );
                       }
                   }
                   return fc::variant( fc::to_double(token.c_str()) );
               case 'a': case 'b': case 'c': case 'd':           case 'f': case 'g': case 'h':
               case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
               case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
               case 'y': case 'z':
               case 'A': case 'B': case 'C': case 'D':           case 'F': case 'G': case 'H':
               case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
               case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
               case 'Y': case 'Z':
               case '_': case '-':           case '+': case '/':
                   if( strict )
                       FC_THROW_EXCEPTION( parse_error_exception, "illegal character '{c}' parsing number", ( "c", c ) );
                   return fc::variant( token );
               default:
                   FC_THROW_EXCEPTION( parse_error_exception, "illegal character '{c}' in number", ( "c", c ) );
           }
       }
   }

   template<typename T, bool strict>
   variant_object objectFromStream( T& in )
   {
      mutable_variant_object obj;
      try
      {
         char c = in.peek();
         if( c != '{' )
            FC_THROW_EXCEPTION( parse_error_exception,
                                     "Expected '{', but read '${char}'",
                                     ("char",string(&c, &c + 1)) );
         in.get();
         skip_white_space(in);
         while( in.peek() != '}' )
         {
            if( in.peek() == ',' )
            {
               in.get();
               continue;
            }
            if( skip_white_space(in) ) continue;
            string key = stringFromStream<T, strict>( in );
            skip_white_space(in);
            if( in.peek() != ':' )
            {
               FC_THROW_EXCEPTION( parse_error_exception, "Expected ':' after key \"${key}\"",
                                        ("key", key) );
            }
            in.get();
            auto val = variant_from_stream<T, strict>( in );

            obj(std::move(key),std::move(val));
            skip_white_space(in);
         }
         if( in.peek() == '}' )
         {
            in.get();
            return obj;
         }
         FC_THROW_EXCEPTION( parse_error_exception, "Expected '}' after ${variant}", ("variant", obj ) );
      }
      catch( const fc::eof_exception& e )
      {
         FC_THROW_EXCEPTION( parse_error_exception, "Unexpected EOF: ${e}", ("e", e.to_detail_string() ) );
      }
      catch( const std::ios_base::failure& e )
      {
         FC_THROW_EXCEPTION( parse_error_exception, "Unexpected EOF: ${e}", ("e", e.what() ) );
      } FC_RETHROW_EXCEPTIONS( warn, "Error parsing object" );
   }

   template<typename T, bool strict>
   variants arrayFromStream( T& in )
   {
      variants ar;
      try
      {
        if( in.peek() != '[' )
           FC_THROW_EXCEPTION( parse_error_exception, "Expected '['" );
        in.get();
        skip_white_space(in);

        while( in.peek() != ']' )
        {
           if( in.peek() == ',' )
           {
              in.get();
              continue;
           }
           if( skip_white_space(in) ) continue;
           ar.push_back( variant_from_stream<T, strict>(in) );
           skip_white_space(in);
        }
        if( in.peek() != ']' )
           FC_THROW_EXCEPTION( parse_error_exception, "Expected ']' after parsing ${variant}",
                                    ("variant", ar) );

        in.get();
      } FC_RETHROW_EXCEPTIONS( warn, "Attempting to parse array ${array}",
                                         ("array", ar ) );
      return ar;
   }

   template<typename T, bool strict>
   variant numberFromStream( T& in )
   {
       fc::string token = tokenFromStream(in);
       variant result = parseNumberOrStr<strict>( token );
       if( strict && !(result.is_int64() || result.is_uint64() || result.is_double()) )
           FC_THROW_EXCEPTION( parse_error_exception, "expected: number" );
       return result;
   }
   
   template<typename T, bool strict>
   variant wordFromStream( T& in )
   {
       fc::string token = tokenFromStream(in);
       
       FC_ASSERT( token.length() > 0 );

       switch( token[0] )
       {
           case 'n':
               if( token == "null" )
                   return variant();
               break;
           case 't':
               if( token == "true" )
                   return variant( true );
               break;
           case 'f':
               if( token == "false" )
                   return variant( false );
               break;
           default:
               break;
       }

       if( !strict )
           return token;

       FC_THROW_EXCEPTION( parse_error_exception, "expected: null|true|false" );
   }
   
   template<typename T, bool strict>
   variant variant_from_stream( T& in )
   {
      skip_white_space(in);
      variant var;
      while( char c = in.peek() )
      {
         switch( c )
         {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
              in.get();
              continue;
            case '"':
              return stringFromStream<T, strict>( in );
            case '{':
              return objectFromStream<T, strict>( in );
            case '[':
              return arrayFromStream<T, strict>( in );
            case '-':
            case '+':
            case '.':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              return numberFromStream<T, strict>( in );
            // null, true, false, or 'warning' / string
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
            case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
            case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
            case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
            case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
            case 'Y': case 'Z':
            case '_':                               case '/':
              return wordFromStream<T, strict>( in );
            case 0x04: // ^D end of transmission
            case EOF:
              FC_THROW_EXCEPTION( eof_exception, "unexpected end of file" );
            default:
              FC_THROW_EXCEPTION( parse_error_exception, "Unexpected char '${c}' in \"${s}\"",
                                 ("c", c)("s", stringFromToken(in)) );
         }
      }
	  return variant();
   }

} } // fc::json_relaxed
