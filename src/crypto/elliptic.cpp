#include <fc/crypto/elliptic.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <openssl/ec.h>
#include <openssl/crypto.h>
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>
#include <openssl/sha.h>
#include <openssl/obj_mac.h>
#include <assert.h>

namespace fc { namespace ecc {
    namespace detail 
    { 
      class public_key_impl
      {
        public:
          public_key_impl()
          :_key(nullptr)
          {
          }
          ~public_key_impl()
          {
            if( _key != nullptr )
            {
              EC_KEY_free(_key);
            }
          }
          public_key_impl( const public_key_impl& cpy )
          {
            _key = cpy._key ? EC_KEY_dup( cpy._key ) : nullptr;
          }
          EC_KEY* _key;
      };
      class private_key_impl
      {
        public:
          private_key_impl()
          :_key(nullptr)
          {
          }
          ~private_key_impl()
          {
            if( _key != nullptr )
            {
              EC_KEY_free(_key);
            }
          }
          private_key_impl( const private_key_impl& cpy )
          {
            _key = cpy._key ? EC_KEY_dup( cpy._key ) : nullptr;
          }
          EC_KEY* _key;
      };
    }
    void * ecies_key_derivation(const void *input, size_t ilen, void *output, size_t *olen) 
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
    int ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *ecsig, const unsigned char *msg, int msglen, int recid, int check)
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

/*
    public_key::public_key()
    :my( new detail::public_key_impl() )
    {
    }

    public_key::public_key( fc::bigint pub_x, fc::bigint pub_y )
    :my( new detail::public_key_impl() )
    {
    }

    public_key::~public_key()
    {
    }
    */

    private_key::private_key()
    {}

    private_key private_key::regenerate( const fc::sha256& secret )
    {
       private_key self;
       self.my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
       if( !self.my->_key ) FC_THROW_EXCEPTION( exception, "Unable to generate EC key" );
      
       BIGNUM* bn = BN_bin2bn( (const unsigned char*)&secret, 32, BN_new() );
       if( bn == NULL ) 
       {
         FC_THROW_EXCEPTION( exception, "unable to create bignum from secret" );
       }

       if( !EC_KEY_regenerate_key(self.my->_key,bn) )
       {
          BN_clear_free(bn);
          FC_THROW_EXCEPTION( exception, "unable to regenerate key" );
       }

       BN_clear_free(bn);
       return self;
    }

    fc::sha256 private_key::get_secret()const
    {
       fc::sha256 sec;
       const BIGNUM* bn = EC_KEY_get0_private_key(my->_key);
       if( bn == NULL )
       {
         FC_THROW_EXCEPTION( exception, "get private key failed" );
       }
       int nbytes = BN_num_bytes(bn);
       BN_bn2bin(bn, &((unsigned char*)&sec)[32-nbytes] );
       return sec;
    }

    private_key private_key::generate()
    {
       private_key self;
       EC_KEY* k = EC_KEY_new_by_curve_name( NID_secp256k1 );
       if( !k ) FC_THROW_EXCEPTION( exception, "Unable to generate EC key" );
       self.my->_key = k;
       if( !EC_KEY_generate_key( self.my->_key ) )
       {
          elog( "key generation error" );
       }

#if 0
          = bigint( EC_KEY_get0_private_key( k );
       EC_POINT* pub   = EC_KEY_get0_public_key( k );
       EC_GROUP* group = EC_KEY_get0_group( k );

       EC_POINT_get_affine_coordinates_GFp( group, pub, self.my->_pub_x.get(), self.my->_pub_y.get(), nullptr/*ctx*/ );

       EC_KEY_free(k);
#endif

       return self;
    }

    signature private_key::sign( const fc::sha256& digest )
    {
        unsigned int buf_len = ECDSA_size(my->_key);
//        fprintf( stderr, "%d  %d\n", buf_len, sizeof(sha256) );
        signature sig;
        assert( buf_len == sizeof(sig) );

        if( !ECDSA_sign( 0, 
                    (const unsigned char*)&digest, sizeof(digest), 
                    (unsigned char*)&sig, &buf_len, my->_key ) )
        {
            fprintf( stderr, "sign error\n");
        }


        return sig;
    }
    bool       public_key::verify( const fc::sha256& digest, const fc::ecc::signature& sig )
    {
      return 1 == ECDSA_verify( 0, (unsigned char*)&digest, sizeof(digest), (unsigned char*)&sig, sizeof(sig), my->_key ); 
    }

    std::vector<char> public_key::serialize()const
    {
      EC_KEY_set_conv_form( my->_key, POINT_CONVERSION_COMPRESSED );
      size_t nbytes = i2o_ECPublicKey( my->_key, nullptr );
      std::vector<char> dat(nbytes);
      char* front = &dat[0];
      i2o_ECPublicKey( my->_key, (unsigned char**)&front  );
      fprintf( stderr, "public key size: %lu\n", nbytes );
      return dat;
      /*
       EC_POINT* pub   = EC_KEY_get0_public_key( my->_key );
       EC_GROUP* group = EC_KEY_get0_group( my->_key );
       EC_POINT_get_affine_coordinates_GFp( group, pub, self.my->_pub_x.get(), self.my->_pub_y.get(), nullptr );
       */
    }
    public_key::public_key()
    {
    }
    public_key::~public_key()
    {
    }
    public_key::public_key( const std::vector<char>& v )
    {
      const char* front = &v[0];
      my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
      my->_key = o2i_ECPublicKey( &my->_key, (const unsigned char**)&front, v.size() );
      if( !my->_key ) 
      {
        fprintf( stderr, "decode error occurred??" );
      }
    }

    bool       private_key::verify( const fc::sha256& digest, const fc::ecc::signature& sig )
    {
      return 1 == ECDSA_verify( 0, (unsigned char*)&digest, sizeof(digest), (unsigned char*)&sig, sizeof(sig), my->_key ); 
    }

    public_key private_key::get_public_key()const
    {

       public_key pub;  
       pub.my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
       EC_KEY_set_public_key( pub.my->_key, EC_KEY_get0_public_key( my->_key ) );
       return pub;
    }

    private_key::private_key( std::vector<char> k )
    {
#if 0
       fc::bigint priv(k); 
       my->_key = EC_KEY_new_by_curve_name( NID_sect283r1 );
       auto k = my->_key;

       if( !k ) FC_THROW_EXCEPTION( exception, "Unable to generate EC key" );

       EC_KEY_set_private_key( my->_key, priv.get() );

       EC_GROUP* group = EC_KEY_get0_group( k );
       EC_POINT* pub   = EC_POINT_new(group);

       fc::bigint x, y;
       EC_POINT_set_affine_coordinates_GFp( group, pub, x.get(), y.get(), nullptr/*ctx*/ );

       bool fail = false;
       fail = EC_KEY_set_private_key( k, pub ) == 0;
       fail = fail | EC_KEY_check_key( k ) == 0;

       EC_POINT_free( pub );

       if( fail ) FC_THROW_EXCEPTION( exception, "Unable to load private key" );
#endif
    }

    fc::sha512 private_key::get_shared_secret( const public_key& other )
    {
      fc::sha512 buf;
      ECDH_compute_key( (unsigned char*)&buf, sizeof(buf), EC_KEY_get0_public_key(other.my->_key), my->_key, ecies_key_derivation );
      return buf;
    }

    private_key::~private_key()
    {
    }

    public_key::public_key( const compact_signature& c, const fc::sha256& digest )
    {
        int nV = c.data[0];
        if (nV<27 || nV>=35)
            FC_THROW_EXCEPTION( exception, "unable to reconstruct public key from signature" );

        ECDSA_SIG *sig = ECDSA_SIG_new();
        BN_bin2bn(&c.data[1],32,sig->r);
        BN_bin2bn(&c.data[33],32,sig->s);

        my->_key = EC_KEY_new_by_curve_name(NID_secp256k1);

        if (nV >= 31)
        {
            EC_KEY_set_conv_form( my->_key, POINT_CONVERSION_COMPRESSED );
            nV -= 4;
            fprintf( stderr, "compressed\n" );
        }

        if (ECDSA_SIG_recover_key_GFp(my->_key, sig, (unsigned char*)&digest, sizeof(digest), nV - 27, 0) == 1)
        {
            ECDSA_SIG_free(sig);
            return;
        }
        ECDSA_SIG_free(sig);
        FC_THROW_EXCEPTION( exception, "unable to reconstruct public key from signature" );
    }

    compact_signature private_key::sign_compact( const fc::sha256& digest )
    {
        ECDSA_SIG *sig = ECDSA_do_sign((unsigned char*)&digest, sizeof(digest), my->_key);

        if (sig==NULL) 
          FC_THROW_EXCEPTION( exception, "Unable to sign" );

        compact_signature csig;

        int nBitsR = BN_num_bits(sig->r);
        int nBitsS = BN_num_bits(sig->s);
        if (nBitsR <= 256 && nBitsS <= 256)
        {
            int nRecId = -1;
            auto my_pub_key = get_public_key().serialize();
            for (int i=0; i<4; i++)
            {
                public_key keyRec;
                keyRec.my->_key = EC_KEY_new_by_curve_name( NID_secp256k1 );
          //      keyRec.fSet = true;
          //      if (fCompressedPubKey) keyRec.SetCompressedPubKey();
                if (ECDSA_SIG_recover_key_GFp(keyRec.my->_key, sig, (unsigned char*)&digest, sizeof(digest), i, 1) == 1)
                {
                    if (keyRec.serialize() == my_pub_key )
                    {
                       nRecId = i;
                       break;
                    }
                }
            }

            if (nRecId == -1)
            FC_THROW_EXCEPTION( exception, "unable to construct recoverable key");
            
            csig.data[0] = nRecId+27+4;//(fCompressedPubKey ? 4 : 0);
            BN_bn2bin(sig->r,&csig.data[33-(nBitsR+7)/8]);
            BN_bn2bin(sig->s,&csig.data[65-(nBitsS+7)/8]);
        }
        ECDSA_SIG_free(sig);
        return csig;
    }

} }
