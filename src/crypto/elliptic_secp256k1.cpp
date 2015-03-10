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
      static void init_lib() {
          static int init_s = 0;
          static int init_o = init_openssl();
          if (!init_s) {
              secp256k1_start(SECP256K1_START_VERIFY | SECP256K1_START_SIGN);
              init_s = 1;
          }
      }

        typedef public_key_data pub_data_type;
        typedef private_key_secret priv_data_type;

        #include "_elliptic_impl.cpp"

        void public_key_impl::free_key()
        {
            if( _key != nullptr )
            {
                delete _key;
                _key = nullptr;
            }
        }

        public_key_data* public_key_impl::dup_key( const public_key_data* cpy )
        {
            return new public_key_data( *cpy );
        }

        void public_key_impl::copy_key( public_key_data* to, const public_key_data* from )
        {
            *to = *from;
        }

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

    public_key public_key::add( const fc::sha256& digest )const
    {
        FC_ASSERT( my->_key != nullptr );
        public_key_data new_key;
        memcpy( new_key.begin(), my->_key->begin(), new_key.size() );
        FC_ASSERT( secp256k1_ec_pubkey_tweak_add( (unsigned char*) new_key.begin(), new_key.size(), (unsigned char*) digest.data() ) );
        return public_key( new_key );
    }

    std::string public_key::to_base58() const
    {
        FC_ASSERT( my->_key != nullptr );
        return to_base58( *my->_key );
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

    public_key_data public_key::serialize()const
    {
        FC_ASSERT( my->_key != nullptr );
        return *my->_key;
    }
    public_key_point_data public_key::serialize_ecc_point()const
    {
      FC_ASSERT( my->_key != nullptr );
      public_key_point_data dat;
      unsigned int pk_len = my->_key->size();
      memcpy( dat.begin(), my->_key->begin(), pk_len );
      FC_ASSERT( secp256k1_ec_pubkey_decompress( (unsigned char *) dat.begin(), (int*) &pk_len ) );
      FC_ASSERT( pk_len == dat.size() );
      return dat;
    }

    public_key::public_key( const public_key_point_data& dat )
    {
      const char* front = &dat.data[0];
      if( *front == 0 ){}
      else
      {
         EC_KEY *key = EC_KEY_new_by_curve_name( NID_secp256k1 );
         key = o2i_ECPublicKey( &key, (const unsigned char**)&front, sizeof(dat) );
         FC_ASSERT( key );
         EC_KEY_set_conv_form( key, POINT_CONVERSION_COMPRESSED );
         my->_key = new public_key_data();
         unsigned char* buffer = (unsigned char*) my->_key->begin();
         i2o_ECPublicKey( key, &buffer ); // FIXME: questionable memory handling
         EC_KEY_free( key );
      }
    }

    public_key::public_key( const public_key_data& dat )
    {
        my->_key = new public_key_data(dat);
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

    public_key::public_key( const compact_signature& c, const fc::sha256& digest, bool check_canonical )
    {
        int nV = c.data[0];
        if (nV<27 || nV>=35)
            FC_THROW_EXCEPTION( exception, "unable to reconstruct public key from signature" );

        if( check_canonical )
        {
            FC_ASSERT( is_canonical( c ), "signature is not canonical" );
        }

        my->_key = new public_key_data();
        unsigned int pk_len;
        FC_ASSERT( secp256k1_ecdsa_recover_compact( (unsigned char*) digest.data(), (unsigned char*) c.begin() + 1, (unsigned char*) my->_key->begin(), (int*) &pk_len, 1, (*c.begin() - 27) & 3 ) );
        FC_ASSERT( pk_len == my->_key->size() );
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
}
}

#include "_elliptic_common.cpp"
