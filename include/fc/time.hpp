#pragma once
#include <stdint.h>
#include <fc/string.hpp>
#include <fc/raw.hpp>
#include <fc/optional.hpp>

namespace fc {
  class microseconds { 
    public:
        explicit microseconds( int64_t c = 0) :_count(c){}
        static microseconds max() { return microseconds(0x7fffffffffffffffll); }
        friend microseconds operator + (const  microseconds& l, const microseconds& r ) { return microseconds(l._count+r._count); }
        friend microseconds operator - (const  microseconds& l, const microseconds& r ) { return microseconds(l._count-r._count); }

        bool operator==(const microseconds& c)const { return _count == c._count; }
        friend bool operator>(const microseconds& a, const microseconds& b){ return a._count > b._count; }
        friend bool operator<(const microseconds& a, const microseconds& b){ return a._count < b._count; }
        microseconds& operator+=(const microseconds& c) { _count += c._count; return *this; }
        int64_t count()const { return _count; }
    private:
        friend class time_point;
        int64_t      _count; 
  };
  inline microseconds seconds( int64_t s ) { return microseconds( s * 1000000 ); }
  inline microseconds milliseconds( int64_t s ) { return microseconds( s * 1000 ); }

  class time_point { 
    public:
        explicit time_point( microseconds e = microseconds() ) :elapsed(e){}
        static time_point now();
        static time_point max() { return time_point( microseconds::max() ); }
        static time_point min() { return time_point();                      }
        operator fc::string()const;
  
        static time_point from_iso_string( const fc::string& s );

        const microseconds& time_since_epoch()const { return elapsed; }
        bool   operator > ( const time_point& t )const                              { return elapsed._count > t.elapsed._count; }
        bool   operator < ( const time_point& t )const                              { return elapsed._count < t.elapsed._count; }
        bool   operator <=( const time_point& t )const                              { return elapsed._count <=t.elapsed._count; }
        bool   operator ==( const time_point& t )const                              { return elapsed._count ==t.elapsed._count; }
        bool   operator !=( const time_point& t )const                              { return elapsed._count !=t.elapsed._count; }
        time_point&  operator += ( const microseconds& m )                          { elapsed+=m; return *this;               }
        friend time_point   operator + ( const time_point& t, const microseconds& m ) { return time_point(t.elapsed+m);         }
        friend time_point   operator - ( const time_point& t, const microseconds& m ) { return time_point(t.elapsed-m);         }
        friend microseconds operator - ( const time_point& t, const time_point& m ) { return microseconds(t.elapsed.count() - m.elapsed.count()); }
    private:
        microseconds elapsed; 
  };

  // forward declare io
  class value;
  void pack( fc::value& , const fc::time_point&  );
  void unpack( const fc::value& , fc::time_point&  );

  namespace raw {
    template<typename Stream, typename T>
    void unpack( Stream& s, fc::time_point& v );
    template<typename Stream, typename T>
    void pack( Stream& s, const fc::time_point& v );
  }

  typedef fc::optional<time_point> otime_point;
}
