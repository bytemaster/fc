#ifndef _FC_TIME_HPP_
#define _FC_TIME_HPP_
#include <stdint.h>

namespace fc {
  class microseconds { 
    public:
        explicit microseconds( int64_t c = 0) :_count(c){}
        static microseconds max() { return microseconds(0x7fffffffffffffffll); }
        friend microseconds operator + (const  microseconds& l, const microseconds& r ) { return microseconds(l._count+r._count); }

        bool operator==(const microseconds& c)const { return _count == c._count; }
        int64_t count()const { return _count; }
    private:
        friend class time_point;
        int64_t      _count; 
  };

  class time_point { 
    public:
        explicit time_point( microseconds e = microseconds() ) :elapsed(e){}
        static time_point now();
        static time_point max() { return time_point( microseconds::max() ); }
        static time_point min() { return time_point();                      }
        const microseconds& time_since_epoch()const { return elapsed; }
        bool   operator > ( const time_point& t )const                              { return elapsed._count > t.elapsed._count; }
        bool   operator < ( const time_point& t )const                              { return elapsed._count < t.elapsed._count; }
        bool   operator <=( const time_point& t )const                              { return elapsed._count <=t.elapsed._count; }
        bool   operator ==( const time_point& t )const                              { return elapsed._count ==t.elapsed._count; }
        bool   operator !=( const time_point& t )const                              { return elapsed._count !=t.elapsed._count; }
        friend time_point   operator + ( const time_point& t, const microseconds& m ) { return time_point(t.elapsed+m);         }
        friend microseconds operator - ( const time_point& t, const time_point& m ) { return microseconds(t.elapsed.count() - m.elapsed.count()); }
    private:
        microseconds elapsed; 
  };
}
#endif // _FC_TIME_HPP_
