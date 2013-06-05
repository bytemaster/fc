#pragma once
#include <fc/crypto/bigint.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/fwd.hpp>
#include <fc/array.hpp>

namespace fc { namespace ecc {
    
    namespace detail 
    { 
      class public_key_impl; 
      class private_key_impl; 
    }

    typedef fc::array<char,72> signature;
    typedef fc::array<unsigned char,65> compact_signature;

    class public_key
    {
        public:
           public_key();
           ~public_key();
           bool verify( const fc::sha256& digest, const signature& sig );

           std::vector<char> serialize();
           public_key( const std::vector<char>& v );
           public_key( const compact_signature& c, const fc::sha256& digest );
        private:
          friend class private_key;
          fc::fwd<detail::public_key_impl,8> my;
    };


    class private_key 
    {
        public:
           private_key();
           private_key( std::vector<char> k );
           ~private_key();

           static private_key generate();
           static private_key regenerate( const fc::sha256& secret );

           fc::sha256 get_secret()const; // get the private key secret

           /**
            *  Given a public key, calculatse a 512 bit shared secret between that
            *  key and this private key.  
            */
           fc::sha512 get_shared_secret( const public_key& pub );

           signature         sign( const fc::sha256& digest );
           compact_signature sign_compact( const fc::sha256& digest );
           bool      verify( const fc::sha256& digest, const signature& sig );

           public_key get_public_key()const;
        private:
           fc::fwd<detail::private_key_impl,8> my;
    };
} } // fc::ecc
