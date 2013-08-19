#include <fc/crypto/openssl.hpp>
namespace  fc 
{
    struct openssl_scope
    {
       openssl_scope()
       {
          ERR_load_crypto_strings(); 
          OpenSSL_add_all_algorithms();
          OPENSSL_config(NULL);
       }
       ~openssl_scope()
       {
          EVP_cleanup();
          ERR_free_strings();
       }
    };
   
    int init_openssl()
    {
        static openssl_scope ossl;
        return 0;
    }
}
