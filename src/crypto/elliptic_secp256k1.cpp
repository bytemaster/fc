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

      static public_key_data empty_key;

      class public_key_impl
      {
        public:
          public_key_impl() : _key(nullptr)
          {
              init_lib();
          }

          public_key_impl( const public_key_impl& cpy )
          {
              init_lib();
              _key = nullptr;
              *this = cpy;
          }

          public_key_impl( public_key_impl&& cpy )
          {
              init_lib();
              _key = nullptr;
              *this = cpy;
          }

          ~public_key_impl()
          {
            if( _key != nullptr )
            {
              delete _key;
              _key = nullptr;
            }
          }

          public_key_impl& operator=( const public_key_impl& pk )
          {
              if (pk._key == nullptr)
              {
                  if (_key != nullptr)
                  {
                      delete _key;
                      _key = nullptr;
                  }
              } else if ( _key == nullptr ) {
                  _key = new public_key_data(*pk._key);
              } else {
                  *_key = *pk._key;
              }
              return *this;
          }

          public_key_impl& operator=( public_key_impl&& pk )
          {
              if (_key != nullptr)
              {
                  delete _key;
              }
              _key = pk._key;
              pk._key = nullptr;
              return *this;
          }

          public_key_data *_key;
      };
      class private_key_impl
      {
        public:
          private_key_impl() : _key(nullptr)
          {
              init_lib();
          }

          private_key_impl( const private_key_impl& cpy )
          {
              init_lib();
              _key = nullptr;
              *this = cpy;
          }

          private_key_impl( private_key_impl&& cpy )
          {
              init_lib();
              _key = nullptr;
              *this = cpy;
          }

          ~private_key_impl()
          {
            if( _key != nullptr )
            {
               delete _key;
              _key = nullptr;
            }
          }

          private_key_impl& operator=( const private_key_impl& pk )
          {
              if (pk._key == nullptr)
              {
                  if (_key != nullptr)
                  {
                      delete _key;
                      _key = nullptr;
                  }
              } else if ( _key == nullptr ) {
                  _key = new private_key_secret(*pk._key);
              } else {
                  *_key = *pk._key;
              }
              return *this;
          }

          private_key_impl& operator=( private_key_impl&& pk )
          {
              if (_key != nullptr)
              {
                  delete _key;
              }
              _key = pk._key;
              pk._key = nullptr;
              return *this;
          }

          private_key_secret *_key;
      };
    }
//    static void * ecies_key_derivation(const void *input, size_t ilen, void *output, size_t *olen)
//    {
//        if (*olen < SHA512_DIGEST_LENGTH) {
//          return NULL;
//        }
//        *olen = SHA512_DIGEST_LENGTH;
//        return (void*)SHA512((const unsigned char*)input, ilen, (unsigned char*)output);
//    }
//
//    // Perform ECDSA key recovery (see SEC1 4.1.6) for curves over (mod p)-fields
//    // recid selects which key is recovered
//    // if check is non-zero, additional checks are performed
//    static int ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *ecsig, const unsigned char *msg, int msglen, int recid, int check)
//    {
//        if (!eckey) FC_THROW_EXCEPTION( exception, "null key" );
//
//        int ret = 0;
//        BN_CTX *ctx = NULL;
//
//        BIGNUM *x = NULL;
//        BIGNUM *e = NULL;
//        BIGNUM *order = NULL;
//        BIGNUM *sor = NULL;
//        BIGNUM *eor = NULL;
//        BIGNUM *field = NULL;
//        EC_POINT *R = NULL;
//        EC_POINT *O = NULL;
//        EC_POINT *Q = NULL;
//        BIGNUM *rr = NULL;
//        BIGNUM *zero = NULL;
//        int n = 0;
//        int i = recid / 2;
//
//        const EC_GROUP *group = EC_KEY_get0_group(eckey);
//        if ((ctx = BN_CTX_new()) == NULL) { ret = -1; goto err; }
//        BN_CTX_start(ctx);
//        order = BN_CTX_get(ctx);
//        if (!EC_GROUP_get_order(group, order, ctx)) { ret = -2; goto err; }
//        x = BN_CTX_get(ctx);
//        if (!BN_copy(x, order)) { ret=-1; goto err; }
//        if (!BN_mul_word(x, i)) { ret=-1; goto err; }
//        if (!BN_add(x, x, ecsig->r)) { ret=-1; goto err; }
//        field = BN_CTX_get(ctx);
//        if (!EC_GROUP_get_curve_GFp(group, field, NULL, NULL, ctx)) { ret=-2; goto err; }
//        if (BN_cmp(x, field) >= 0) { ret=0; goto err; }
//        if ((R = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
//        if (!EC_POINT_set_compressed_coordinates_GFp(group, R, x, recid % 2, ctx)) { ret=0; goto err; }
//        if (check)
//        {
//            if ((O = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
//            if (!EC_POINT_mul(group, O, NULL, R, order, ctx)) { ret=-2; goto err; }
//            if (!EC_POINT_is_at_infinity(group, O)) { ret = 0; goto err; }
//        }
//        if ((Q = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
//        n = EC_GROUP_get_degree(group);
//        e = BN_CTX_get(ctx);
//        if (!BN_bin2bn(msg, msglen, e)) { ret=-1; goto err; }
//        if (8*msglen > n) BN_rshift(e, e, 8-(n & 7));
//        zero = BN_CTX_get(ctx);
//        if (!BN_zero(zero)) { ret=-1; goto err; }
//        if (!BN_mod_sub(e, zero, e, order, ctx)) { ret=-1; goto err; }
//        rr = BN_CTX_get(ctx);
//        if (!BN_mod_inverse(rr, ecsig->r, order, ctx)) { ret=-1; goto err; }
//        sor = BN_CTX_get(ctx);
//        if (!BN_mod_mul(sor, ecsig->s, rr, order, ctx)) { ret=-1; goto err; }
//        eor = BN_CTX_get(ctx);
//        if (!BN_mod_mul(eor, e, rr, order, ctx)) { ret=-1; goto err; }
//        if (!EC_POINT_mul(group, Q, eor, R, sor, ctx)) { ret=-2; goto err; }
//        if (!EC_KEY_set_public_key(eckey, Q)) { ret=-2; goto err; }
//
//        ret = 1;
//
//    err:
//        if (ctx) {
//            BN_CTX_end(ctx);
//            BN_CTX_free(ctx);
//        }
//        if (R != NULL) EC_POINT_free(R);
//        if (O != NULL) EC_POINT_free(O);
//        if (Q != NULL) EC_POINT_free(Q);
//        return ret;
//    }
//
//
//    int static inline EC_KEY_regenerate_key(EC_KEY *eckey, const BIGNUM *priv_key)
//    {
//        int ok = 0;
//        BN_CTX *ctx = NULL;
//        EC_POINT *pub_key = NULL;
//
//        if (!eckey) return 0;
//
//        const EC_GROUP *group = EC_KEY_get0_group(eckey);
//
//        if ((ctx = BN_CTX_new()) == NULL)
//        goto err;
//
//        pub_key = EC_POINT_new(group);
//
//        if (pub_key == NULL)
//        goto err;
//
//        if (!EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, ctx))
//        goto err;
//
//        EC_KEY_set_private_key(eckey,priv_key);
//        EC_KEY_set_public_key(eckey,pub_key);
//
//        ok = 1;
//
//        err:
//
//        if (pub_key) EC_POINT_free(pub_key);
//        if (ctx != NULL) BN_CTX_free(ctx);
//
//        return(ok);
//    }

    public_key public_key::from_key_data( const public_key_data &data ) {
        return public_key(data);
    }

//    public_key public_key::mult( const fc::sha256& digest )const
//    {
//        FC_ASSERT( my->_key != nullptr );
//        public_key_data new_key;
//        memcpy( new_key.begin(), my->_key->begin(), new_key.size() );
//        FC_ASSERT( secp256k1_ec_pubkey_tweak_mul( (unsigned char*) new_key.begin(), new_key.size(), (unsigned char*) digest.data() ) );
//        return public_key( new_key );
//    }

    bool       public_key::valid()const
    {
      return my->_key != nullptr;
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

    private_key::private_key()
    {}

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

    public_key::public_key()
    {
    }

    public_key::~public_key()
    {
    }

    public_key::public_key( const public_key_point_data& dat )
    {
      const char* front = &dat.data[0];
      if( *front == 0 ){}
      else
      {
         EC_KEY *key = o2i_ECPublicKey( nullptr, (const unsigned char**)&front, sizeof(dat) );
         FC_ASSERT( key );
         EC_KEY_set_conv_form( key, POINT_CONVERSION_COMPRESSED );
         my->_key = new public_key_data();
         i2o_ECPublicKey( key, (unsigned char**)&my->_key->data );
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

    private_key::~private_key()
    {
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

    private_key& private_key::operator=( private_key&& pk )
   {
     my = std::move(pk.my);
     return *this;
   }
   public_key::public_key( const public_key& pk )
   :my(pk.my)
   {
   }
   public_key::public_key( public_key&& pk )
   :my( std::move(pk.my) )
   {
   }
   private_key::private_key( const private_key& pk )
   :my(pk.my)
   {
   }
   private_key::private_key( private_key&& pk )
   :my( std::move( pk.my) )
   {
   }

   public_key& public_key::operator=( public_key&& pk )
   {
     my = std::move(pk.my);
     return *this;
   }
   public_key& public_key::operator=( const public_key& pk )
   {
     my = pk.my;
     return *this;
   }
   private_key& private_key::operator=( const private_key& pk )
   {
     my = pk.my;
     return *this;
   }
}
}
