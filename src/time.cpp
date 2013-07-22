#include <fc/time.hpp>
#include <fc/variant.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
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
      
      return boost::posix_time::to_iso_string( boost::posix_time::from_time_t(tt) + boost::posix_time::microseconds( elapsed._count % 1000000 ) ); 
  }
  time_point time_point::from_iso_string( const fc::string& s ) {
     auto pt = boost::posix_time::from_iso_string(s);
     //return fc::time_point(fc::seconds( (pt - boost::posix_time::from_time_t(0)).total_seconds() ));
     return fc::time_point(fc::microseconds( (pt - boost::posix_time::from_time_t(0)).total_microseconds() ));
  }
  void to_variant( const fc::time_point& t, variant& v ) {
    v = fc::string(t);
  }
  void from_variant( const fc::variant& v, fc::time_point& t ) {
    t = fc::time_point::from_iso_string(v.as_string());
  }
  void to_variant( const fc::time_point_sec& t, variant& v ) {
    v = fc::string(fc::time_point(t));
  }
  void from_variant( const fc::variant& v, fc::time_point_sec& t ) {
    t = fc::time_point::from_iso_string(v.as_string());
  }
}
