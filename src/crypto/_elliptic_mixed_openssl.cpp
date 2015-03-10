namespace detail
{
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
