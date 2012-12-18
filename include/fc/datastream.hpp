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
      datastream( T start, size_t s )
      :m_start(start),m_pos(start),m_end(start+s){};
      
      
      inline void skip( size_t s ){ m_pos += s; }
      inline bool read( char* d, size_t s ) {
        if( m_end - m_pos >= (size_t)s ) {
          memcpy( d, m_pos, s );
          m_pos += s;
          return true;
        }
        FC_THROW_REPORT( "Attempt to read ${bytes_past} bytes beyond end of buffer with size ${buffer_size} ", 
                        fc::value().set("bytes_past",int64_t(-((m_end-m_pos) - s))).set("buffer_size", int64_t(m_end-m_start)) );
        return false;
      }
      
      inline bool write( const char* d, size_t s ) {
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
      inline bool     seekp(size_t p) { m_pos = m_start + p; return m_pos <= m_end; }
      inline size_t   tellp()const      { return m_pos - m_start;                     }
      inline size_t   remaining()const  { return m_end - m_pos;                       }
    private:
      T m_start;
      T m_pos;
      T m_end;
};

template<>
struct datastream<size_t> {
  datastream( size_t init_size = 0):m_size(init_size){};
  inline bool     skip( size_t s )                 { m_size += s; return true; }
  inline bool     write( const char* d,size_t s ) { m_size += s; return true; }
  inline bool     put(char c)                        { ++m_size; return  true; }
  inline bool     valid()const                       { return true;              }
  inline bool     seekp(size_t p)                  { m_size = p;  return true; }
  inline size_t   tellp()const                       { return m_size;            }
  inline size_t   remaining()const                   { return 0;                 }
private:
  size_t m_size;
};

/*
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const size_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return *this;
}
template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, size_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return *this;
}
*/
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const int32_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}
template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, int32_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const uint32_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, uint32_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const int64_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, int64_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const uint64_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, uint64_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const int16_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, int16_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const uint16_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, uint16_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const int8_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, int8_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const uint8_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, uint8_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}


} // namespace fc

