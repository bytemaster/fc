#pragma once
#include <fc/crypto/bigint.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/fwd.hpp>
#include <fc/array.hpp>
#include <fc/io/raw_fwd.hpp>

namespace fc { 

  namespace ecc {
    namespace detail 
    { 
      class public_key_impl; 
      class private_key_impl; 
    }

    typedef fc::array<char,33>          public_key_data;
    typedef fc::array<char,72>          signature;
    typedef fc::array<unsigned char,65> compact_signature;

    /**
     *  @class public_key
     *  @brief contains only the public point of an elliptic curve key.
     */
    class public_key
    {
        public:
           public_key();
           public_key(const public_key& k);
           ~public_key();
           bool verify( const fc::sha256& digest, const signature& sig );
           public_key_data serialize()const;

           operator public_key_data()const { return serialize(); }

           public_key( const public_key_data& v );
           public_key( const compact_signature& c, const fc::sha256& digest );

           bool valid()const;
           public_key mult( const fc::sha256& offset );
           public_key add( const fc::sha256& offset )const;

           public_key( public_key&& pk );
           public_key& operator=( public_key&& pk );
           public_key& operator=( const public_key& pk );

           inline friend bool operator==( const public_key& a, const public_key& b )
           {
            return a.serialize() == b.serialize();
           }
        private:
          friend class private_key;
          fc::fwd<detail::public_key_impl,8> my;
    };

    /**
     *  @class private_key
     *  @brief an elliptic curve private key.
     */
    class private_key 
    {
        public:
           private_key();
           private_key( private_key&& pk );
           private_key( const private_key& pk );
           ~private_key();

           private_key& operator=( private_key&& pk );
           private_key& operator=( const private_key& pk );

           static private_key generate();
           static private_key regenerate( const fc::sha256& secret );

           /**
            *  This method of generation enables creating a new private key in a deterministic manner relative to
            *  an initial seed.   A public_key created from the seed can be multiplied by the offset to calculate 
            *  the new public key without having to know the private key.
            */
           static private_key generate_from_seed( const fc::sha256& seed, const fc::sha256& offset = fc::sha256() );

           fc::sha256 get_secret()const; // get the private key secret

           /**
            *  Given a public key, calculatse a 512 bit shared secret between that
            *  key and this private key.  
            */
           fc::sha512 get_shared_secret( const public_key& pub )const;

           signature         sign( const fc::sha256& digest );
           compact_signature sign_compact( const fc::sha256& digest )const;
           bool              verify( const fc::sha256& digest, const signature& sig );

           public_key get_public_key()const;
        private:
           fc::fwd<detail::private_key_impl,8> my;
    };
  } // namespace ecc
  void to_variant( const ecc::private_key& var,  variant& vo );
  void from_variant( const variant& var,  ecc::private_key& vo );
  void to_variant( const ecc::public_key& var,  variant& vo );
  void from_variant( const variant& var,  ecc::public_key& vo );

  namespace raw
  {
      template<typename Stream>
      void unpack( Stream& s, fc::ecc::public_key& pk)
      {
          ecc::public_key_data ser;
          fc::raw::unpack(s,ser);
          pk = fc::ecc::public_key( ser );
      }

      template<typename Stream>
      void pack( Stream& s, const fc::ecc::public_key& pk)
      {
          fc::raw::pack( s, pk.serialize() );
      }

      template<typename Stream>
      void unpack( Stream& s, fc::ecc::private_key& pk)
      {
          fc::sha256 sec;
          unpack( s, sec );
          pk = ecc::private_key::regenerate(sec);
      }

      template<typename Stream>
      void pack( Stream& s, const fc::ecc::private_key& pk)
      {
          fc::raw::pack( s, pk.get_secret() );
      }

  } // namespace raw

} // namespace fc 
