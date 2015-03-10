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
