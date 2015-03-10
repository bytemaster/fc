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
        typedef EC_KEY priv_data_type;

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
                EC_KEY_free(_key);
                _key = nullptr;
            }
        }

        EC_KEY* private_key_impl::dup_key( const EC_KEY* cpy )
        {
            return EC_KEY_dup( cpy );
        }

        void private_key_impl::copy_key( EC_KEY* to, const EC_KEY* from )
        {
            EC_KEY_copy( to, from );
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



    static void * ecies_key_derivation(const void *input, size_t ilen, void *output, size_t *olen)
    {
        if (*olen < SHA512_DIGEST_LENGTH) {
          return NULL;
        }
        *olen = SHA512_DIGEST_LENGTH;
        return (void*)SHA512((const unsigned char*)input, ilen, (unsigned char*)output);
    }

    // Perform ECDSA key recovery (see SEC1 4.1.6) for curves over (mod p)-fields
    // recid selects which key is recovered
    // if check is non-zero, additional checks are performed
    static int ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *ecsig, const unsigned char *msg, int msglen, int recid, int check)
    {
        if (!eckey) FC_THROW_EXCEPTION( exception, "null key" );

        int ret = 0;
        BN_CTX *ctx = NULL;

        BIGNUM *x = NULL;
        BIGNUM *e = NULL;
        BIGNUM *order = NULL;
        BIGNUM *sor = NULL;
        BIGNUM *eor = NULL;
        BIGNUM *field = NULL;
        EC_POINT *R = NULL;
        EC_POINT *O = NULL;
        EC_POINT *Q = NULL;
        BIGNUM *rr = NULL;
        BIGNUM *zero = NULL;
        int n = 0;
        int i = recid / 2;

        const EC_GROUP *group = EC_KEY_get0_group(eckey);
        if ((ctx = BN_CTX_new()) == NULL) { ret = -1; goto err; }
        BN_CTX_start(ctx);
        order = BN_CTX_get(ctx);
        if (!EC_GROUP_get_order(group, order, ctx)) { ret = -2; goto err; }
        x = BN_CTX_get(ctx);
        if (!BN_copy(x, order)) { ret=-1; goto err; }
        if (!BN_mul_word(x, i)) { ret=-1; goto err; }
        if (!BN_add(x, x, ecsig->r)) { ret=-1; goto err; }
        field = BN_CTX_get(ctx);
        if (!EC_GROUP_get_curve_GFp(group, field, NULL, NULL, ctx)) { ret=-2; goto err; }
        if (BN_cmp(x, field) >= 0) { ret=0; goto err; }
        if ((R = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
        if (!EC_POINT_set_compressed_coordinates_GFp(group, R, x, recid % 2, ctx)) { ret=0; goto err; }
        if (check)
        {
            if ((O = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
            if (!EC_POINT_mul(group, O, NULL, R, order, ctx)) { ret=-2; goto err; }
            if (!EC_POINT_is_at_infinity(group, O)) { ret = 0; goto err; }
        }
        if ((Q = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
        n = EC_GROUP_get_degree(group);
        e = BN_CTX_get(ctx);
        if (!BN_bin2bn(msg, msglen, e)) { ret=-1; goto err; }
        if (8*msglen > n) BN_rshift(e, e, 8-(n & 7));
        zero = BN_CTX_get(ctx);
        if (!BN_zero(zero)) { ret=-1; goto err; }
        if (!BN_mod_sub(e, zero, e, order, ctx)) { ret=-1; goto err; }
        rr = BN_CTX_get(ctx);
        if (!BN_mod_inverse(rr, ecsig->r, order, ctx)) { ret=-1; goto err; }
        sor = BN_CTX_get(ctx);
        if (!BN_mod_mul(sor, ecsig->s, rr, order, ctx)) { ret=-1; goto err; }
        eor = BN_CTX_get(ctx);
        if (!BN_mod_mul(eor, e, rr, order, ctx)) { ret=-1; goto err; }
        if (!EC_POINT_mul(group, Q, eor, R, sor, ctx)) { ret=-2; goto err; }
        if (!EC_KEY_set_public_key(eckey, Q)) { ret=-2; goto err; }

        ret = 1;

    err:
        if (ctx) {
            BN_CTX_end(ctx);
            BN_CTX_free(ctx);
        }
        if (R != NULL) EC_POINT_free(R);
        if (O != NULL) EC_POINT_free(O);
        if (Q != NULL) EC_POINT_free(Q);
        return ret;
    }

    int static inline EC_KEY_regenerate_key(EC_KEY *eckey, const BIGNUM *priv_key)
    {
        int ok = 0;
        BN_CTX *ctx = NULL;
        EC_POINT *pub_key = NULL;

        if (!eckey) return 0;

        const EC_GROUP *group = EC_KEY_get0_group(eckey);

        if ((ctx = BN_CTX_new()) == NULL)
        goto err;

        pub_key = EC_POINT_new(group);

        if (pub_key == NULL)
        goto err;

        if (!EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, ctx))
        goto err;

        EC_KEY_set_private_key(eckey,priv_key);
        EC_KEY_set_public_key(eckey,pub_key);

        ok = 1;

        err:

        if (pub_key) EC_POINT_free(pub_key);
        if (ctx != NULL) BN_CTX_free(ctx);

        return(ok);
    }

    private_key private_key::regenerate( const fc::sha256& secret )
    {
       private_key self;
       self.my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
       if( !self.my->_key ) FC_THROW_EXCEPTION( exception, "Unable to generate EC key" );

       ssl_bignum bn;
       BN_bin2bn( (const unsigned char*)&secret, 32, bn );

       if( !EC_KEY_regenerate_key(self.my->_key,bn) )
       {
          FC_THROW_EXCEPTION( exception, "unable to regenerate key" );
       }
       return self;
    }

    fc::sha256 private_key::get_secret()const
    {
       return get_secret( my->_key );
    }

    private_key::private_key( EC_KEY* k )
    {
       my->_key = k;
    }

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

    compact_signature private_key::sign_compact( const fc::sha256& digest )const
    {
       try {
        FC_ASSERT( my->_key != nullptr );
        auto my_pub_key = get_public_key().serialize(); // just for good measure
        //ECDSA_SIG *sig = ECDSA_do_sign((unsigned char*)&digest, sizeof(digest), my->_key);
        public_key_data key_data;
        while( true )
        {
           ecdsa_sig sig = ECDSA_do_sign((unsigned char*)&digest, sizeof(digest), my->_key);

           if (sig==nullptr)
             FC_THROW_EXCEPTION( exception, "Unable to sign" );

           compact_signature csig;
          // memset( csig.data, 0, sizeof(csig) );

           int nBitsR = BN_num_bits(sig->r);
           int nBitsS = BN_num_bits(sig->s);
           if (nBitsR <= 256 && nBitsS <= 256)
           {
               int nRecId = -1;
               EC_KEY* key = EC_KEY_new_by_curve_name( NID_secp256k1 );
               FC_ASSERT( key );
               EC_KEY_set_conv_form( key, POINT_CONVERSION_COMPRESSED );
               for (int i=0; i<4; i++)
               {
                   if (ECDSA_SIG_recover_key_GFp(key, sig, (unsigned char*)&digest, sizeof(digest), i, 1) == 1)
                   {
                       unsigned char* buffer = (unsigned char*) key_data.begin();
                       i2o_ECPublicKey( key, &buffer ); // FIXME: questionable memory handling
                       if ( key_data == my_pub_key )
                       {
                           nRecId = i;
                           break;
                       }
                   }
               }
               EC_KEY_free( key );

               if (nRecId == -1)
               {
                 FC_THROW_EXCEPTION( exception, "unable to construct recoverable key");
               }
               unsigned char* result = nullptr;
               auto bytes = i2d_ECDSA_SIG( sig, &result );
               auto lenR = result[3];
               auto lenS = result[5+lenR];
               //idump( (result[0])(result[1])(result[2])(result[3])(result[3+lenR])(result[4+lenR])(bytes)(lenR)(lenS) );
               if( lenR != 32 ) { free(result); continue; }
               if( lenS != 32 ) { free(result); continue; }
               //idump( (33-(nBitsR+7)/8) );
               //idump( (65-(nBitsS+7)/8) );
               //idump( (sizeof(csig) ) );
               memcpy( &csig.data[1], &result[4], lenR );
               memcpy( &csig.data[33], &result[6+lenR], lenS );
               //idump( (csig.data[33]) );
               //idump( (csig.data[1]) );
               free(result);
               //idump( (nRecId) );
               csig.data[0] = nRecId+27+4;//(fCompressedPubKey ? 4 : 0);
               /*
               idump( (csig) );
               auto rlen = BN_bn2bin(sig->r,&csig.data[33-(nBitsR+7)/8]);
               auto slen = BN_bn2bin(sig->s,&csig.data[65-(nBitsS+7)/8]);
               idump( (rlen)(slen) );
               */
           }
           return csig;
        } // while true
      } FC_RETHROW_EXCEPTIONS( warn, "sign ${digest}", ("digest", digest)("private_key",*this) );
    }
}
}

#include "_elliptic_common.cpp"
