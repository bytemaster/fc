#include <fc/crypto/elliptic.hpp>

#include <fc/crypto/base58.hpp>
#include <fc/crypto/openssl.hpp>

#include <fc/fwd_impl.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>

#include <assert.h>
#include <secp256k1.h>

namespace fc { namespace ecc {
    namespace detail
    {
        static void init_lib();

        typedef public_key_data pub_data_type;
        typedef private_key_secret priv_data_type;

        #include "_elliptic_impl.cpp"

        void private_key_impl::free_key()
        {
            if( _key != nullptr )
            {
                delete _key;
                _key = nullptr;
            }
        }

        private_key_secret* private_key_impl::dup_key( const private_key_secret* cpy )
        {
            return new private_key_secret( *cpy );
        }

        void private_key_impl::copy_key( private_key_secret* to, const private_key_secret* from )
        {
            *to = *from;
        }
    }

    private_key private_key::regenerate( const fc::sha256& secret )
    {
       private_key self;
       self.my->_key = new private_key_secret(secret);
       return self;
    }

    fc::sha256 private_key::get_secret()const
    {
        FC_ASSERT( my->_key != nullptr );
        return *my->_key;
    }

    private_key::private_key( EC_KEY* k )
    {
       my->_key = new private_key_secret( get_secret( k ) );
       EC_KEY_free(k);
    }

    public_key private_key::get_public_key()const
    {
       FC_ASSERT( my->_key != nullptr );
       public_key_data pub;
       unsigned int pk_len;
       FC_ASSERT( secp256k1_ec_pubkey_create( (unsigned char*) pub.begin(), (int*) &pk_len, (unsigned char*) my->_key->data(), 1 ) );
       FC_ASSERT( pk_len == pub.size() );
       return public_key(pub);
    }

    fc::sha512 private_key::get_shared_secret( const public_key& other )const
    {
      FC_ASSERT( my->_key != nullptr );
      FC_ASSERT( other.my->_key != nullptr );
      public_key_data pub(*other.my->_key);
      FC_ASSERT( secp256k1_ec_pubkey_tweak_mul( (unsigned char*) pub.begin(), pub.size(), (unsigned char*) my->_key->data() ) );
//      ECDH_compute_key( (unsigned char*)&buf, sizeof(buf), EC_KEY_get0_public_key(other.my->_key), my->_key, ecies_key_derivation );
      return fc::sha512::hash( pub.begin() + 1, pub.size() - 1 );
    }

    compact_signature private_key::sign_compact( const fc::sha256& digest )const
    {
        FC_ASSERT( my->_key != nullptr );
        compact_signature result;
        int recid;
        do
        {
            FC_ASSERT( secp256k1_ecdsa_sign_compact( (unsigned char*) digest.data(), (unsigned char*) result.begin() + 1, (unsigned char*) my->_key->data(), NULL, NULL, &recid ));
        } while( !public_key::is_canonical( result ) );
        result.begin()[0] = 27 + 4 + recid;
        return result;
    }

    #include "_elliptic_mixed_secp256k1.cpp"

} }

#include "_elliptic_common.cpp"
