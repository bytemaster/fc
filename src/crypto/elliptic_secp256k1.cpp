#include <fc/crypto/elliptic.hpp>

#include <fc/crypto/base58.hpp>
#include <fc/crypto/hmac.hpp>
#include <fc/crypto/openssl.hpp>
#include <fc/crypto/sha512.hpp>

#include <fc/fwd_impl.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>

#include <assert.h>
#include <secp256k1.h>

#include "_elliptic_impl_priv.hpp"

#define BTC_EXT_PUB_MAGIC   (0x0488B21E)
#define BTC_EXT_PRIV_MAGIC  (0x0488ADE4)

namespace fc { namespace ecc {
    namespace detail
    {
        const secp256k1_context_t* _get_context() {
            static secp256k1_context_t* ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_RANGEPROOF | SECP256K1_CONTEXT_COMMIT );
            return ctx;
        }

        void _init_lib() {
            static const secp256k1_context_t* ctx = _get_context();
            static int init_o = init_openssl();
            (void)ctx;
        }

        class public_key_impl
        {
            public:
                public_key_impl() BOOST_NOEXCEPT
                {
                    _init_lib();
                }

                public_key_impl( const public_key_impl& cpy ) BOOST_NOEXCEPT
                    : _key( cpy._key )
                {
                    _init_lib();
                }

                public_key_data _key;
        };
    }

    static const public_key_data empty_pub;
    static const private_key_secret empty_priv;

    fc::sha512 private_key::get_shared_secret( const public_key& other )const
    {
      FC_ASSERT( my->_key != empty_priv );
      FC_ASSERT( other.my->_key != empty_pub );
      public_key_data pub(other.my->_key);
      FC_ASSERT( secp256k1_ec_pubkey_tweak_mul( detail::_get_context(), (unsigned char*) pub.begin(), pub.size(), (unsigned char*) my->_key.data() ) );
      return fc::sha512::hash( pub.begin() + 1, pub.size() - 1 );
    }


    public_key::public_key() {}

    public_key::public_key( const public_key &pk ) : my( pk.my ) {}

    public_key::public_key( public_key &&pk ) : my( std::move( pk.my ) ) {}

    public_key::~public_key() {}

    public_key& public_key::operator=( const public_key& pk )
    {
        my = pk.my;
        return *this;
    }

    public_key& public_key::operator=( public_key&& pk )
    {
        my = pk.my;
        return *this;
    }

    bool public_key::valid()const
    {
      return my->_key != empty_pub;
    }

    public_key public_key::add( const fc::sha256& digest )const
    {
        FC_ASSERT( my->_key != empty_pub );
        public_key_data new_key;
        memcpy( new_key.begin(), my->_key.begin(), new_key.size() );
        FC_ASSERT( secp256k1_ec_pubkey_tweak_add( detail::_get_context(), (unsigned char*) new_key.begin(), new_key.size(), (unsigned char*) digest.data() ) );
        return public_key( new_key );
    }

    std::string public_key::to_base58() const
    {
        FC_ASSERT( my->_key != empty_pub );
        return to_base58( my->_key );
    }

    public_key_data public_key::serialize()const
    {
        FC_ASSERT( my->_key != empty_pub );
        return my->_key;
    }

    public_key_point_data public_key::serialize_ecc_point()const
    {
        FC_ASSERT( my->_key != empty_pub );
        public_key_point_data dat;
        unsigned int pk_len = my->_key.size();
        memcpy( dat.begin(), my->_key.begin(), pk_len );
        FC_ASSERT( secp256k1_ec_pubkey_decompress( detail::_get_context(), (unsigned char *) dat.begin(), (int*) &pk_len ) );
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
            unsigned char* buffer = (unsigned char*) my->_key.begin();
            i2o_ECPublicKey( key, &buffer ); // FIXME: questionable memory handling
            EC_KEY_free( key );
        }
    }

    public_key::public_key( const public_key_data& dat )
    {
        my->_key = dat;
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

        unsigned int pk_len;
        FC_ASSERT( secp256k1_ecdsa_recover_compact( detail::_get_context(), (unsigned char*) digest.data(), (unsigned char*) c.begin() + 1, (unsigned char*) my->_key.begin(), (int*) &pk_len, 1, (*c.begin() - 27) & 3 ) );
        FC_ASSERT( pk_len == my->_key.size() );
    }

    static fc::sha256 _left( const fc::sha512& v )
    {
        fc::sha256 result;
        memcpy( result.data(), v.data(), 32 );
        return result;
    }

    static fc::sha256 _right( const fc::sha512& v )
    {
        fc::sha256 result;
        memcpy( result.data(), v.data() + 32, 32 );
        return result;
    }

    typedef fc::array<char,37> chr37;

    static void _put( unsigned char** dest, unsigned int i)
    {
        *(*dest)++ = (i >> 24) & 0xff;
        *(*dest)++ = (i >> 16) & 0xff;
        *(*dest)++ = (i >>  8) & 0xff;
        *(*dest)++ =  i        & 0xff;
    }

    static unsigned int _get( unsigned char** src )
    {
        unsigned int result = *(*src)++ << 24;
        result |= *(*src)++ << 16;
        result |= *(*src)++ <<  8;
        result |= *(*src)++;
        return result;
    }

    static chr37 _derive_message( char first, const char* key32, int i )
    {
        chr37 result;
        unsigned char* dest = (unsigned char*) result.begin();
        *dest++ = first;
        memcpy( dest, key32, 32 ); dest += 32;
        _put( &dest, i );
        return result;
    }

    static chr37 _derive_message( const public_key_data& key, int i )
    {
        return _derive_message( *key.begin(), key.begin() + 1, i );
    }

    static chr37 _derive_message( const private_key_secret& key, int i )
    {
        return _derive_message( 0, key.data(), i );
    }

    static fc::string _to_base58( const extended_key_data& key )
    {
        char buffer[key.size() + 4];
        memcpy( buffer, key.begin(), key.size() );
        fc::sha256 double_hash = fc::sha256::hash( fc::sha256::hash( key.begin(), key.size() ));
        memcpy( buffer + key.size(), double_hash.data(), 4 );
        return fc::to_base58( buffer, sizeof(buffer) );
    }

    static void _parse_extended_data( unsigned char* buffer, fc::string base58 )
    {
        memset( buffer, 0, 78 );
        std::vector<char> decoded = fc::from_base58( base58 );
        unsigned int i = 0;
        for ( char c : decoded )
        {
            if ( i >= 78 || i > decoded.size() - 4 ) { break; }
            buffer[i++] = c;
        }
    }

    typedef hmac<fc::sha512> hmac_sha512;

    extended_public_key::extended_public_key( const public_key& k, const fc::sha256& c,
                                              int child, int parent, uint8_t depth )
        : public_key(k), c(c), child_num(child), parent_fp(parent), depth(depth) { }

    extended_public_key extended_public_key::derive_child(int i) const
    {
        FC_ASSERT( !(i&0x80000000), "Can't derive hardened public key!" );
        return derive_normal_child(i);
    }

    extended_public_key extended_public_key::derive_normal_child(int i) const
    {
        hmac_sha512 mac;
        public_key_data key = serialize();
        const chr37 data = _derive_message( key, i );
        fc::sha512 l = mac.digest( c.data(), c.data_size(), data.begin(), data.size() );
        fc::sha256 left = _left(l);
        FC_ASSERT( secp256k1_ec_pubkey_tweak_add( detail::_get_context(), (unsigned char*) key.begin(), key.size(), (unsigned char*) left.data() ) > 0 );
        extended_public_key result( key, _right(l), i, fingerprint(), depth + 1 );
        return result;
    }

    extended_key_data extended_public_key::serialize_extended() const
    {
        extended_key_data result;
        unsigned char* dest = (unsigned char*) result.begin();
        _put( &dest, BTC_EXT_PUB_MAGIC );
        *dest++ = depth;
        _put( &dest, parent_fp );
        _put( &dest, child_num );
        memcpy( dest, c.data(), c.data_size() ); dest += 32;
        public_key_data key = serialize();
        memcpy( dest, key.begin(), key.size() );
        return result;
    }

    fc::string extended_public_key::str() const
    {
        return _to_base58( serialize_extended() );
    }

    extended_public_key extended_public_key::from_base58( const fc::string& base58 )
    {
        unsigned char buffer[78];
        unsigned char* ptr = buffer;
        _parse_extended_data( buffer, base58 );
        FC_ASSERT( _get( &ptr ) == BTC_EXT_PUB_MAGIC, "Invalid extended private key" );
        uint8_t d = *ptr++;
        int fp = _get( &ptr );
        int cn = _get( &ptr );
        fc::sha256 chain;
        memcpy( chain.data(), ptr, chain.data_size() ); ptr += chain.data_size();
        public_key_data key;
        memcpy( key.begin(), ptr, key.size() );
        return extended_public_key( key, chain, cn, fp, d );
    }

    extended_private_key::extended_private_key( const private_key& k, const sha256& c,
                                                int child, int parent, uint8_t depth )
        : private_key(k), c(c), child_num(child), parent_fp(parent), depth(depth) { }

    extended_public_key extended_private_key::get_extended_public_key() const
    {
        return extended_public_key( get_public_key(), c, child_num, parent_fp, depth );
    }

    extended_private_key extended_private_key::derive_child(int i) const
    {
        return i < 0 ? derive_hardened_child(i) : derive_normal_child(i);
    }

    extended_private_key extended_private_key::derive_normal_child(int i) const
    {
        const chr37 data = _derive_message( get_public_key().serialize(), i );
        hmac_sha512 mac;
        fc::sha512 l = mac.digest( c.data(), c.data_size(), data.begin(), data.size() );
        return private_derive_rest( l, i );
    }

    extended_private_key extended_private_key::derive_hardened_child(int i) const
    {
        hmac_sha512 mac;
        private_key_secret key = get_secret();
        const chr37 data = _derive_message( key, i );
        fc::sha512 l = mac.digest( c.data(), c.data_size(), data.begin(), data.size() );
        return private_derive_rest( l, i );
    }

    extended_private_key extended_private_key::private_derive_rest( const fc::sha512& hash,
                                                                    int i) const
    {
        fc::sha256 left = _left(hash);
        FC_ASSERT( secp256k1_ec_privkey_tweak_add( detail::_get_context(), (unsigned char*) left.data(), (unsigned char*) get_secret().data() ) > 0 );
        extended_private_key result( private_key::regenerate( left ), _right(hash),
                                     i, fingerprint(), depth + 1 );
        return result;
    }

    extended_key_data extended_private_key::serialize_extended() const
    {
        extended_key_data result;
        unsigned char* dest = (unsigned char*) result.begin();
        _put( &dest, BTC_EXT_PRIV_MAGIC );
        *dest++ = depth;
        _put( &dest, parent_fp );
        _put( &dest, child_num );
        memcpy( dest, c.data(), c.data_size() ); dest += 32;
        *dest++ = 0;
        private_key_secret key = get_secret();
        memcpy( dest, key.data(), key.data_size() );
        return result;
    }

    fc::string extended_private_key::str() const
    {
        return _to_base58( serialize_extended() );
    }

    extended_private_key extended_private_key::from_base58( const fc::string& base58 )
    {
        unsigned char buffer[78];
        unsigned char* ptr = buffer;
        _parse_extended_data( buffer, base58 );
        FC_ASSERT( _get( &ptr ) == BTC_EXT_PRIV_MAGIC, "Invalid extended private key" );
        uint8_t d = *ptr++;
        int fp = _get( &ptr );
        int cn = _get( &ptr );
        fc::sha256 chain;
        memcpy( chain.data(), ptr, chain.data_size() ); ptr += chain.data_size();
        ptr++;
        private_key_secret key;
        memcpy( key.data(), ptr, key.data_size() );
        return extended_private_key( private_key::regenerate(key), chain, cn, fp, d );
    }

    extended_private_key extended_private_key::generate_master( const fc::string& seed )
    {
        return generate_master( seed.c_str(), seed.size() );
    }

    extended_private_key extended_private_key::generate_master( const char* seed, uint32_t seed_len )
    {
        hmac_sha512 mac;
        fc::sha512 hash = mac.digest( "Bitcoin seed", 12, seed, seed_len );
        extended_private_key result( private_key::regenerate( _left(hash) ), _right(hash) );
        return result;
    }


     commitment_type blind( const blind_factor_type& blind, uint64_t value )
     {
        commitment_type result;
        FC_ASSERT( secp256k1_pedersen_commit( detail::_get_context(), (unsigned char*)&result, (unsigned char*)&blind, value ) );
        return result;
     }

     blind_factor_type blind_sum( const std::vector<blind_factor_type>& blinds_in, uint32_t non_neg )
     {
        blind_factor_type result;
        std::vector<const unsigned char*> blinds(blinds_in.size());
        for( uint32_t i = 0; i < blinds_in.size(); ++i ) blinds[i] = (const unsigned char*)&blinds_in[i];
        FC_ASSERT( secp256k1_pedersen_blind_sum( detail::_get_context(), (unsigned char*)&result, blinds.data(), blinds_in.size(), non_neg ) );
        return result;
     }

     /**  verifies taht commnits + neg_commits + excess == 0 */
     bool            verify_sum( const std::vector<commitment_type>& commits_in, const std::vector<commitment_type>& neg_commits_in, int64_t excess )
     {
        std::vector<const unsigned char*> commits(commits_in.size());
        for( uint32_t i = 0; i < commits_in.size(); ++i ) commits[i] = (const unsigned char*)&commits_in[i];
        std::vector<const unsigned char*> neg_commits(neg_commits_in.size());
        for( uint32_t i = 0; i < neg_commits_in.size(); ++i ) neg_commits[i] = (const unsigned char*)&neg_commits_in[i];

        return secp256k1_pedersen_verify_tally( detail::_get_context(), commits.data(), commits.size(), neg_commits.data(), neg_commits.size(), excess  );
     }

     bool            verify_range( uint64_t& min_val, uint64_t& max_val, const commitment_type& commit, const std::vector<char>& proof )
     {
        return secp256k1_rangeproof_verify( detail::_get_context(), &min_val, &max_val, (const unsigned char*)&commit, (const unsigned char*)proof.data(), proof.size() );
     }

     std::vector<char>    range_proof_sign( uint64_t min_value, 
                                       const commitment_type& commit, 
                                       const blind_factor_type& commit_blind, 
                                       const blind_factor_type& nonce,
                                       int8_t base10_exp,
                                       uint8_t min_bits,
                                       uint64_t actual_value
                                     )
     {
        int proof_len = 5134; 
        std::vector<char> proof(proof_len);

        FC_ASSERT( secp256k1_rangeproof_sign( detail::_get_context(), 
                                              (unsigned char*)proof.data(), 
                                              &proof_len, min_value, 
                                              (const unsigned char*)&commit, 
                                              (const unsigned char*)&commit_blind, 
                                              (const unsigned char*)&nonce, 
                                              base10_exp, min_bits, actual_value ) );
        proof.resize(proof_len);
        return proof;
     }


     bool            verify_range_proof_rewind( blind_factor_type& blind_out,
                                                uint64_t& value_out,
                                                string& message_out, 
                                                const blind_factor_type& nonce,
                                                uint64_t& min_val, 
                                                uint64_t& max_val, 
                                                commitment_type commit, 
                                                const std::vector<char>& proof )
     {
        char msg[4096];
        int  mlen = 0;
        FC_ASSERT( secp256k1_rangeproof_rewind( detail::_get_context(), 
                                                (unsigned char*)&blind_out,
                                                &value_out,
                                                (unsigned char*)msg,
                                                &mlen,
                                                (const unsigned char*)&nonce,
                                                &min_val,
                                                &max_val,
                                                (const unsigned char*)&commit,
                                                (const unsigned char*)proof.data(),
                                                proof.size() ) );

        message_out = std::string( msg, mlen );
        return true;
     }

     range_proof_info range_get_info( const std::vector<char>& proof )
     {
        range_proof_info result;
        FC_ASSERT( secp256k1_rangeproof_info( detail::_get_context(), 
                                              (int*)&result.exp, 
                                              (int*)&result.mantissa, 
                                              (uint64_t*)&result.min_value, 
                                              (uint64_t*)&result.max_value, 
                                              (const unsigned char*)proof.data(), 
                                              (int)proof.size() ) );

        return result;
     }


} }
