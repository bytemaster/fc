#pragma once
#include <fc/crypto/elliptic.hpp>

/* public_key_impl implementation based on openssl
 * used by mixed + openssl
 */

namespace fc { namespace ecc { namespace detail {

void _init_lib();

class public_key_impl
{
    public:
        public_key_impl() noexcept;
        public_key_impl( const public_key_impl& cpy ) noexcept;
        public_key_impl( public_key_impl&& cpy ) noexcept;
        ~public_key_impl() noexcept;

        public_key_impl& operator=( const public_key_impl& pk ) noexcept;

        public_key_impl& operator=( public_key_impl&& pk ) noexcept;

        static int ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *ecsig, const unsigned char *msg, int msglen, int recid, int check);

        EC_KEY* _key = nullptr;

    private:
        void free_key() noexcept;
};

}}}
