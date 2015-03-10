#include <fc/crypto/elliptic.hpp>

#include <fc/crypto/base58.hpp>
#include <fc/crypto/openssl.hpp>

#include <fc/fwd_impl.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>

#include <assert.h>

namespace fc { namespace ecc {
    namespace detail
    {
        static void init_lib()
        {
            static int init = init_openssl();
        }

        typedef EC_KEY pub_data_type;
        typedef EC_KEY priv_data_type;

        #include "_elliptic_impl.cpp"

        void public_key_impl::free_key()
        {
            if( _key != nullptr )
            {
                EC_KEY_free(_key);
                _key = nullptr;
            }
        }

        EC_KEY* public_key_impl::dup_key( const EC_KEY* cpy )
        {
            return EC_KEY_dup( cpy );
        }

        void public_key_impl::copy_key( EC_KEY* to, const EC_KEY* from )
        {
            EC_KEY_copy( to, from );
        }
    }

    #include "_elliptic_mixed_openssl.cpp"

    /* WARNING! This implementation is broken, it is actually equivalent to
     * public_key::add()!
     */
//    public_key public_key::mult( const fc::sha256& digest ) const
//    {
//        // get point from this public key
//        const EC_POINT* master_pub   = EC_KEY_get0_public_key( my->_key );
//        ec_group group(EC_GROUP_new_by_curve_name(NID_secp256k1));
//
//        ssl_bignum z;
//        BN_bin2bn((unsigned char*)&digest, sizeof(digest), z);
//
//        // multiply by digest
//        ssl_bignum one;
//        BN_one(one);
//        bn_ctx ctx(BN_CTX_new());
//
//        ec_point result(EC_POINT_new(group));
//        EC_POINT_mul(group, result, z, master_pub, one, ctx);
//
//        public_key rtn;
//        rtn.my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
//        EC_KEY_set_public_key(rtn.my->_key,result);
//
//        return rtn;
//    }
    public_key public_key::add( const fc::sha256& digest )const
    {
      try {
        ec_group group(EC_GROUP_new_by_curve_name(NID_secp256k1));
        bn_ctx ctx(BN_CTX_new());

        fc::bigint digest_bi( (char*)&digest, sizeof(digest) );

        ssl_bignum order;
        EC_GROUP_get_order(group, order, ctx);
        if( digest_bi > fc::bigint(order) )
        {
          FC_THROW_EXCEPTION( exception, "digest > group order" );
        }


        public_key digest_key = private_key::regenerate(digest).get_public_key();
        const EC_POINT* digest_point   = EC_KEY_get0_public_key( digest_key.my->_key );

        // get point from this public key
        const EC_POINT* master_pub   = EC_KEY_get0_public_key( my->_key );

//        ssl_bignum z;
//        BN_bin2bn((unsigned char*)&digest, sizeof(digest), z);

        // multiply by digest
//        ssl_bignum one;
//        BN_one(one);

        ec_point result(EC_POINT_new(group));
        EC_POINT_add(group, result, digest_point, master_pub, ctx);

        if (EC_POINT_is_at_infinity(group, result))
        {
          FC_THROW_EXCEPTION( exception, "point at  infinity" );
        }


        public_key rtn;
        rtn.my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
        EC_KEY_set_public_key(rtn.my->_key,result);
        return rtn;
      } FC_RETHROW_EXCEPTIONS( debug, "digest: ${digest}", ("digest",digest) );
    }

    std::string public_key::to_base58() const
    {
      public_key_data key = serialize();
      return to_base58( key );
    }

//    signature private_key::sign( const fc::sha256& digest )const
//    {
//        unsigned int buf_len = ECDSA_size(my->_key);
////        fprintf( stderr, "%d  %d\n", buf_len, sizeof(sha256) );
//        signature sig;
//        assert( buf_len == sizeof(sig) );
//
//        if( !ECDSA_sign( 0,
//                    (const unsigned char*)&digest, sizeof(digest),
//                    (unsigned char*)&sig, &buf_len, my->_key ) )
//        {
//            FC_THROW_EXCEPTION( exception, "signing error" );
//        }
//
//
//        return sig;
//    }
//    bool       public_key::verify( const fc::sha256& digest, const fc::ecc::signature& sig )
//    {
//      return 1 == ECDSA_verify( 0, (unsigned char*)&digest, sizeof(digest), (unsigned char*)&sig, sizeof(sig), my->_key );
//    }

    public_key_data public_key::serialize()const
    {
      public_key_data dat;
      if( !my->_key ) return dat;
      EC_KEY_set_conv_form( my->_key, POINT_CONVERSION_COMPRESSED );
      /*size_t nbytes = i2o_ECPublicKey( my->_key, nullptr ); */
      /*assert( nbytes == 33 )*/
      char* front = &dat.data[0];
      i2o_ECPublicKey( my->_key, (unsigned char**)&front ); // FIXME: questionable memory handling
      return dat;
      /*
       EC_POINT* pub   = EC_KEY_get0_public_key( my->_key );
       EC_GROUP* group = EC_KEY_get0_group( my->_key );
       EC_POINT_get_affine_coordinates_GFp( group, pub, self.my->_pub_x.get(), self.my->_pub_y.get(), nullptr );
       */
    }
    public_key_point_data public_key::serialize_ecc_point()const
    {
      public_key_point_data dat;
      if( !my->_key ) return dat;
      EC_KEY_set_conv_form( my->_key, POINT_CONVERSION_UNCOMPRESSED );
      char* front = &dat.data[0];
      i2o_ECPublicKey( my->_key, (unsigned char**)&front ); // FIXME: questionable memory handling
      return dat;
    }

    public_key::public_key( const public_key_point_data& dat )
    {
      const char* front = &dat.data[0];
      if( *front == 0 ){}
      else
      {
         my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
         my->_key = o2i_ECPublicKey( &my->_key, (const unsigned char**)&front, sizeof(dat)  );
         if( !my->_key )
         {
           FC_THROW_EXCEPTION( exception, "error decoding public key", ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
         }
      }
    }
    public_key::public_key( const public_key_data& dat )
    {
      const char* front = &dat.data[0];
      if( *front == 0 ){}
      else
      {
         my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
         my->_key = o2i_ECPublicKey( &my->_key, (const unsigned char**)&front, sizeof(public_key_data) );
         if( !my->_key )
         {
           FC_THROW_EXCEPTION( exception, "error decoding public key", ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
         }
      }
    }

//    bool       private_key::verify( const fc::sha256& digest, const fc::ecc::signature& sig )
//    {
//      return 1 == ECDSA_verify( 0, (unsigned char*)&digest, sizeof(digest), (unsigned char*)&sig, sizeof(sig), my->_key );
//    }

    public_key private_key::get_public_key()const
    {
       public_key pub;
       pub.my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
       EC_KEY_set_public_key( pub.my->_key, EC_KEY_get0_public_key( my->_key ) );
       return pub;
    }


    fc::sha512 private_key::get_shared_secret( const public_key& other )const
    {
      FC_ASSERT( my->_key != nullptr );
      FC_ASSERT( other.my->_key != nullptr );
      fc::sha512 buf;
      ECDH_compute_key( (unsigned char*)&buf, sizeof(buf), EC_KEY_get0_public_key(other.my->_key), my->_key, ecies_key_derivation );
      return buf;
    }

    public_key::public_key( const compact_signature& c, const fc::sha256& digest, bool check_canonical )
    {
        int nV = c.data[0];
        if (nV<27 || nV>=35)
            FC_THROW_EXCEPTION( exception, "unable to reconstruct public key from signature" );

        ECDSA_SIG *sig = ECDSA_SIG_new();
        BN_bin2bn(&c.data[1],32,sig->r);
        BN_bin2bn(&c.data[33],32,sig->s);

        if( check_canonical )
        {
            FC_ASSERT( is_canonical( c ), "signature is not canonical" );
        }

        my->_key = EC_KEY_new_by_curve_name(NID_secp256k1);

        if (nV >= 31)
        {
            EC_KEY_set_conv_form( my->_key, POINT_CONVERSION_COMPRESSED );
            nV -= 4;
//            fprintf( stderr, "compressed\n" );
        }

        if (ECDSA_SIG_recover_key_GFp(my->_key, sig, (unsigned char*)&digest, sizeof(digest), nV - 27, 0) == 1)
        {
            ECDSA_SIG_free(sig);
            return;
        }
        ECDSA_SIG_free(sig);
        FC_THROW_EXCEPTION( exception, "unable to reconstruct public key from signature" );
    }

    #include "_elliptic_mixed_openssl.cpp"
} }

#include "_elliptic_common.cpp"
