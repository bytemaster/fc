#pragma once
#include <fc/utility.hpp>
#include <fc/error_report.hpp>
#include <string.h>
#include <stdint.h>

namespace fc {

/**
 *  The purpose of this datastream is to provide a fast, effecient, means
 *  of calculating the amount of data "about to be written" and then
 *  writing it.  This means having two modes of operation, "test run" where
 *  you call the entire pack sequence calculating the size, and then 
 *  actually packing it after doing a single allocation.
 */
template<typename T>
struct datastream {
      datastream( T start, uint32_t s )
      :m_start(start),m_pos(start),m_end(start+s){};
      
      template<typename DATA>
      inline datastream& operator<<(const DATA& d) {
        static_assert( !fc::is_class<DATA>::type::value, "no serialization defined" );
        write( (const char*)&d, sizeof(d) );
        return *this;
      }
      
      template<typename DATA>
      inline datastream& operator>>(DATA& d) {
        static_assert( !fc::is_class<DATA>::type::value, "no serialization defined" );
        read((char*)&d, sizeof(d) );
        return *this;
      }
      
      inline void skip( uint32_t s ){ m_pos += s; }
      inline bool read( char* d, uint32_t s ) {
        if( m_end - m_pos >= (int32_t)s ) {
          memcpy( d, m_pos, s );
          m_pos += s;
          return true;
        }
        FC_THROW_REPORT( "Attempt to read ${bytes_past} bytes beyond end of buffer with size ${buffer_size} ", 
                        fc::value().set("bytes_past",int64_t(-((m_end-m_pos) - s))).set("buffer_size", int64_t(m_end-m_start)) );
        return false;
      }
      
      inline bool write( const char* d, uint32_t s ) {
        if( m_end - m_pos >= (int32_t)s ) {
          memcpy( m_pos, d, s );
          m_pos += s;
          return true;
        }
        FC_THROW_REPORT( "Attempt to write ${bytes_past} bytes beyond end of buffer with size ${buffer_size} ", 
                        fc::value().set("bytes_past",int64_t(-((m_end-m_pos) - s))).set("buffer_size", int64_t(m_end-m_start)) );
        return false;
      }
      
      inline bool   put(char c) { 
        if( m_pos < m_end ) {
          *m_pos = c; 
          ++m_pos; 
          return true;
        }
        FC_THROW_REPORT( "Attempt to write ${bytes_past} bytes beyond end of buffer with size ${buffer_size} ", 
                        fc::value().set("bytes_past",int64_t(-((m_end-m_pos) - 1))).set("buffer_size", int64_t(m_end-m_start)) );
        return  false;
      }
      
      inline bool   get( unsigned char& c ) { return get( *(char*)&c ); }
      inline bool   get( char& c ) {
        if( m_pos < m_end ) {
          c = *m_pos;
          ++m_pos; 
          return true;
        }
        FC_THROW_REPORT( "Attempt to read ${bytes_past} bytes beyond end of buffer of size ${buffer_size} ", 
                        fc::value().set("bytes_past",int64_t(-((m_end-m_pos) - 1))).set("buffer_size", int64_t(m_end-m_start)) );
        ++m_pos; 
        return  false;
      }
      
      T               pos()const        { return m_pos;                               }
      inline bool     valid()const      { return m_pos <= m_end && m_pos >= m_start;  }
      inline bool     seekp(uint32_t p) { m_pos = m_start + p; return m_pos <= m_end; }
      inline uint32_t tellp()const      { return m_pos - m_start;                     }
      inline uint32_t remaining()const  { return m_end - m_pos;                       }
    private:
      T m_start;
      T m_pos;
      T m_end;
};

template<>
struct datastream<size_t> {
  datastream( size_t init_size = 0):m_size(init_size){};
  template<typename DATA>
  inline datastream& operator<<(const DATA& d) {
    static_assert( !fc::is_class<DATA>::type::value, "no serialzation defined" );
    m_size += sizeof(d);
    return *this;
  }
  inline bool     skip( uint32_t s )                 { m_size += s; return true; }
  inline bool     write( const char* d, uint32_t s ) { m_size += s; return true; }
  inline bool     put(char c)                        { ++m_size; return  true; }
  inline bool     valid()const                       { return true;              }
  inline bool     seekp(uint32_t p)                  { m_size = p;  return true; }
  inline uint32_t tellp()const                       { return m_size;            }
  inline uint32_t remaining()const                   { return 0;                 }
private:
  uint32_t m_size;
};


} // namespace fc

