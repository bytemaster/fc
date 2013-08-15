#pragma once
#include <stdint.h>
#include <fc/string.hpp>
#include <fc/optional.hpp>

namespace fc {
  class microseconds { 
    public:
        explicit microseconds( int64_t c = 0) :_count(c){}
        static microseconds maximum() { return microseconds(0x7fffffffffffffffll); }
        friend microseconds operator + (const  microseconds& l, const microseconds& r ) { return microseconds(l._count+r._count); }
        friend microseconds operator - (const  microseconds& l, const microseconds& r ) { return microseconds(l._count-r._count); }


        bool operator==(const microseconds& c)const { return _count == c._count; }
        friend bool operator>(const microseconds& a, const microseconds& b){ return a._count > b._count; }
        friend bool operator>=(const microseconds& a, const microseconds& b){ return a._count >= b._count; }
        friend bool operator<(const microseconds& a, const microseconds& b){ return a._count < b._count; }
        friend bool operator<=(const microseconds& a, const microseconds& b){ return a._count <= b._count; }
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
        static time_point maximum() { return time_point( microseconds::maximum() ); }
        static time_point (min)() { return time_point();                      }
        operator fc::string()const;
  
        static time_point from_iso_string( const fc::string& s );

        const microseconds& time_since_epoch()const { return elapsed; }
        bool   operator > ( const time_point& t )const                              { return elapsed._count > t.elapsed._count; }
        bool   operator >=( const time_point& t )const                              { return elapsed._count >=t.elapsed._count; }
        bool   operator < ( const time_point& t )const                              { return elapsed._count < t.elapsed._count; }
        bool   operator <=( const time_point& t )const                              { return elapsed._count <=t.elapsed._count; }
        bool   operator ==( const time_point& t )const                              { return elapsed._count ==t.elapsed._count; }
        bool   operator !=( const time_point& t )const                              { return elapsed._count !=t.elapsed._count; }
        time_point&  operator += ( const microseconds& m )                          { elapsed+=m; return *this;               }
        friend time_point   operator + ( const time_point& t, const microseconds& m ) { return time_point(t.elapsed+m);         }
        friend time_point   operator - ( const time_point& t, const microseconds& m ) { return time_point(t.elapsed-m);         }
        friend microseconds operator - ( const time_point& t, const time_point& m )   { return microseconds(t.elapsed.count() - m.elapsed.count()); }
    private:
        microseconds elapsed; 
  };

  /**
   *  A lower resolution time_point accurate only to seconds from 1970
   */
  class time_point_sec 
  {
    public:
        time_point_sec()
        :utc_seconds(0){}

        explicit time_point_sec(uint32_t seconds )
        :utc_seconds(seconds){}

        time_point_sec( const time_point& t )
        :utc_seconds( t.time_since_epoch().count() / 1000000ll ){}

        operator time_point()const { return time_point( fc::seconds( utc_seconds) ); }
        uint32_t sec_since_epoch()const { return utc_seconds; }

        time_point_sec operator = ( const fc::time_point& t )
        {
          utc_seconds = t.time_since_epoch().count() / 1000000ll;
          return *this;
        }
        friend bool   operator < ( const time_point_sec& a, const time_point_sec& b )  { return a.utc_seconds < b.utc_seconds; }
        friend bool   operator > ( const time_point_sec& a, const time_point_sec& b )  { return a.utc_seconds > b.utc_seconds; }
        friend bool   operator == ( const time_point_sec& a, const time_point_sec& b ) { return a.utc_seconds == b.utc_seconds; }

    private:
        uint32_t utc_seconds;
  };

  typedef fc::optional<time_point> otime_point;
}
