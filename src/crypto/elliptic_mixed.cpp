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
        typedef EC_KEY priv_data_type;

        #include "_elliptic_impl.cpp"
    }

    #include "_elliptic_mixed_openssl.cpp"

    public_key private_key::get_public_key()const
    {
        public_key_data data;
        EC_KEY_set_conv_form( my->_key, POINT_CONVERSION_COMPRESSED );
        unsigned char* buffer = (unsigned char*) data.begin();
        i2o_ECPublicKey( my->_key, &buffer ); // FIXME: questionable memory handling
        return public_key( data );
    }

    fc::sha512 private_key::get_shared_secret( const public_key& other )const
    {
      FC_ASSERT( my->_key != nullptr );
      FC_ASSERT( other.my->_key != nullptr );
      fc::sha512 buf;
      EC_KEY* key = EC_KEY_new_by_curve_name( NID_secp256k1 );
      const unsigned char* buffer = (const unsigned char*) other.my->_key->begin();
      o2i_ECPublicKey( &key, &buffer, sizeof(*other.my->_key) );
      ECDH_compute_key( (unsigned char*)&buf, sizeof(buf), EC_KEY_get0_public_key(key), my->_key, ecies_key_derivation );
      EC_KEY_free(key);
      return buf;
    }

    #include "_elliptic_mixed_secp256k1.cpp"
} }

#include "_elliptic_common.cpp"
