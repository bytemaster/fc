#include <fc/value_cast.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fc/time.hpp>
#include <sstream>

namespace fc {
  namespace bch = boost::chrono;
  time_point time_point::now() {
     return time_point(microseconds(bch::duration_cast<bch::microseconds>(bch::system_clock::now().time_since_epoch()).count()));
  }
  time_point::operator fc::string()const {
      bch::system_clock::time_point tp; 
      tp += bch::microseconds( elapsed._count);
      time_t tt = bch::system_clock::to_time_t(tp);
      return boost::posix_time::to_iso_string( boost::posix_time::from_time_t(tt) + boost::posix_time::microseconds( elapsed._count - tt*1000000 ));
  }
  time_point time_point::from_iso_string( const fc::string& s ) {
     auto pt = boost::posix_time::from_iso_string(s);
     return fc::time_point(fc::seconds( (pt - boost::posix_time::from_time_t(0)).total_seconds() ));
  }
  class value;
  void pack( fc::value&  v, const fc::time_point& t ) {
    v = fc::string(t);
  }
  void unpack( const fc::value& v, fc::time_point& t ) {
    t = fc::time_point::from_iso_string(v.cast<fc::string>());
  }
}
