#include <fc/crypto/elliptic.hpp>
#include <bts/address.hpp>
#include <iostream>

int main( int argc, char** argv )
{
   std::string  pass(argv[1]);
   fc::sha256   h = fc::sha256::hash( pass.c_str(), pass.size() );
   fc::ecc::private_key priv = fc::ecc::private_key::generate_from_seed(h);
   fc::ecc::public_key  pub  = priv.get_public_key();
   std::cerr<<"oroginal master pubkey1: "<<std::string(bts::address(pub))<<"\n";

   pass += "1";
   fc::sha256   h2            = fc::sha256::hash( pass.c_str(), pass.size() );
   fc::ecc::public_key  pub1  = pub.mult( h2 );
   fc::ecc::private_key priv1 = fc::ecc::private_key::generate_from_seed(h, h2);

   std::cerr<<"master pubkey: "<<std::string(bts::address(pub))<<"\n";
   std::cerr<<"derived pubkey1: "<<std::string(bts::address(pub1))<<"\n";
   std::cerr<<"actual pubkey1: "<<std::string(bts::address(priv1.get_public_key()))<<"\n";

  return 0;
}
