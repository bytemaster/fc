#include <fc/crypto/elliptic.hpp>
#include <fc/exception/exception.hpp>
#include <iostream>

int main( int argc, char** argv )
{
   for( uint32_t i = 0; i < 3000; ++ i )
   {
   try {
   FC_ASSERT( argc > 1 );

   std::string  pass(argv[1]);
   fc::sha256   h = fc::sha256::hash( pass.c_str(), pass.size() );
   fc::ecc::private_key priv = fc::ecc::private_key::generate_from_seed(h);
   fc::ecc::public_key  pub  = priv.get_public_key();

   pass += "1";
   fc::sha256   h2            = fc::sha256::hash( pass.c_str(), pass.size() );
   fc::ecc::public_key  pub1  = pub.mult( h2 );
   fc::ecc::private_key priv1 = fc::ecc::private_key::generate_from_seed(h, h2);

   auto sig = priv.sign_compact( h );
   auto recover = fc::ecc::public_key( sig, h );
   FC_ASSERT( recover == priv.get_public_key() );
   } 
   catch ( const fc::exception& e )
   {
      edump( (e.to_detail_string()) );
   }
   }


  return 0;
}
