#include <fc/ip.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace fc { namespace ip {

  address::address( uint32_t ip )
  :_ip(ip){}

  address::address( const fc::string& s ) {
    _ip = boost::asio::ip::address_v4::from_string(s.c_str()).to_ulong();
  }

  address& address::operator=( const fc::string& s ) {
    _ip = boost::asio::ip::address_v4::from_string(s.c_str()).to_ulong();
    return *this;
  }

  address::operator fc::string()const {
    return boost::asio::ip::address_v4(_ip).to_string().c_str();
  }


  endpoint::endpoint()
  :_port(0){}
  endpoint::endpoint(const address& a, uint16_t p)
  :_port(p),_ip(a){}

  uint16_t       endpoint::port()const    { return _port; }
  const address& endpoint::get_address()const { return _ip;   }

  endpoint endpoint::from_string( const string& s ) {
    endpoint ep;
    const std::string& st = reinterpret_cast<const std::string&>(s);
    auto pos = st.find(':');
    ep._ip   = boost::asio::ip::address_v4::from_string(st.substr( 0, pos ) ).to_ulong();
    ep._port = boost::lexical_cast<uint16_t>( st.substr( pos+1, s.size() ) );
    return ep;
  }

  endpoint::operator string()const {
    return string(_ip) + ':' + boost::lexical_cast<uint16_t>(_port);
  }

} } 
