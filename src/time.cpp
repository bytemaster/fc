#include <fc/time.hpp>
#include <fc/variant.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <fc/string.hpp>
#include <fc/io/sstream.hpp>
#include <fc/exception/exception.hpp>

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
  time_point time_point::from_iso_string( const fc::string& s ) 
  {
    try
    {
      auto pt = boost::posix_time::from_iso_string(s);
      //return fc::time_point(fc::seconds( (pt - boost::posix_time::from_time_t(0)).total_seconds() ));
      return fc::time_point(fc::microseconds( (pt - boost::posix_time::from_time_t(0)).total_microseconds() ));
    }
    FC_RETHROW_EXCEPTIONS(warn, "unable to convert ISO-formatted string to fc::time_point")
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

  // inspired by show_date_relative() in git's date.c
  string get_approximate_relative_time_string(const time_point_sec& event_time) {
    time_point_sec now_in_sec(time_point::now());
    if (event_time > now_in_sec)
      return "in the future";
    stringstream result;
    uint32_t seconds_ago = now_in_sec.sec_since_epoch() - event_time.sec_since_epoch();
    if (seconds_ago < 90)
    {
      result << seconds_ago << " second" << (seconds_ago > 1 ? "s" : "") << " ago";
      return result.str();
    }
    uint32_t minutes_ago = (seconds_ago + 30) / 60;
    if (minutes_ago < 90)
    {
      result << minutes_ago << " minute" << (minutes_ago > 1 ? "s" : "") << " ago";
      return result.str();
    }
    uint32_t hours_ago = (minutes_ago + 30) / 60;
    if (hours_ago < 90)
    {
      result << hours_ago << " hour" << (hours_ago > 1 ? "s" : "") << " ago";
      return result.str();
    }
    uint32_t days_ago = (hours_ago + 12) / 24;
    if (days_ago < 90)
    {
      result << days_ago << " day" << (days_ago > 1 ? "s" : "") << " ago";
      return result.str();
    }
    uint32_t weeks_ago = (days_ago + 3) / 7;
    if (weeks_ago < 70)
    {
      result << weeks_ago << " week" << (weeks_ago > 1 ? "s" : "") << " ago";
      return result.str();
    }
    uint32_t months_ago = (days_ago + 15) / 30;
    if (months_ago < 12)
    {
      result << months_ago << " month" << (months_ago > 1 ? "s" : "") << " ago";
      return result.str();
    }
    uint32_t years_ago = days_ago / 365;
    result << years_ago << " year" << (months_ago > 1 ? "s" : "");
    if (months_ago < 12 * 5)
    {
      uint32_t leftover_days = days_ago - (years_ago * 365);
      uint32_t leftover_months = (leftover_days + 15) / 30;
      if (leftover_months)
        result << leftover_months <<  " month" << (months_ago > 1 ? "s" : "");
    }
    result << " ago";
    return result.str();
  }
  string get_approximate_relative_time_string(const time_point& event_time) {
    return get_approximate_relative_time_string(time_point_sec(event_time));
  }

  void to_variant( const microseconds& input_microseconds,  variant& output_variant )
  {
    output_variant = input_microseconds.count();
  }
  void from_variant( const variant& input_variant,  microseconds& output_microseconds )
  {
    output_microseconds = microseconds(input_variant.as_int64());
  }

} //namespace fc
