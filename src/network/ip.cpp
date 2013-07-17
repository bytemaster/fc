#include <fc/network/ip.hpp>
#include <fc/crypto/city.hpp>
#include <fc/variant.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace fc { namespace ip {

  address::address( uint32_t ip )
  :_ip(ip){}

  address::address( const fc::string& s ) {
    _ip = boost::asio::ip::address_v4::from_string(s.c_str()).to_ulong();
  }

  bool operator==( const address& a, const address& b ) {
    return uint32_t(a) == uint32_t(b);
  }
  bool operator!=( const address& a, const address& b ) {
    return uint32_t(a) != uint32_t(b);
  }

  address& address::operator=( const fc::string& s ) {
    _ip = boost::asio::ip::address_v4::from_string(s.c_str()).to_ulong();
    return *this;
  }

  address::operator fc::string()const {
    return boost::asio::ip::address_v4(_ip).to_string().c_str();
  }
  address::operator uint32_t()const {
    return _ip;
  }


  endpoint::endpoint()
  :_port(0){  }
  endpoint::endpoint(const address& a, uint16_t p)
  :_port(p),_ip(a){}

  bool operator==( const endpoint& a, const endpoint& b ) {
    return a._port == b._port  && a._ip == b._ip;
  }
  bool operator!=( const endpoint& a, const endpoint& b ) {
    return a._port != b._port || a._ip != b._ip;
  }

  bool operator< ( const endpoint& a, const endpoint& b )
  {
     return  uint32_t(a.get_address()) < uint32_t(b.get_address()) ||
             (uint32_t(a.get_address()) == uint32_t(b.get_address()) &&
              uint32_t(a.port()) < uint32_t(b.port()));
  }

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
    return string(_ip) + ':' + fc::string(boost::lexical_cast<std::string>(_port).c_str());
  }

}  // namespace ip

  void to_variant( const ip::endpoint& var,  variant& vo )
  {
      vo = fc::string(var);
  }
  void from_variant( const variant& var,  ip::endpoint& vo )
  {
     vo = ip::endpoint::from_string(var.as_string());
  }

  void to_variant( const ip::address& var,  variant& vo )
  {
    vo = fc::string(var);
  }
  void from_variant( const variant& var,  ip::address& vo )
  {
    vo = ip::address(var.as_string());
  }

} 
namespace std
{
    size_t hash<fc::ip::endpoint>::operator()( const fc::ip::endpoint& e )const
    {
        return fc::city_hash64( (char*)&e, sizeof(e) );
    }
}
