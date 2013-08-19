#include <fc/crypto/sha512.hpp>
#include <fc/crypto/openssl.hpp>
#include <fc/exception/exception.hpp>

namespace fc {

static int init = init_openssl();

/** example method from wiki.opensslfoundation.com */
int aes_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
              unsigned char *iv, unsigned char *ciphertext)
{
    evp_cipher_ctx ctx( EVP_CIPHER_CTX_new() );

    int len = 0;
    int ciphertext_len = 0;

    /* Create and initialise the context */
    if(!ctx)
    {
        FC_THROW_EXCEPTION( exception, "error allocating evp cipher context", 
                           ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
    }

    /* Initialise the encryption operation. IMPORTANT - ensure you use a key
    *    * and IV size appropriate for your cipher
    *       * In this example we are using 256 bit AES (i.e. a 256 bit key). The
    *          * IV size for *most* modes is the same as the block size. For AES this
    *             * is 128 bits */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        FC_THROW_EXCEPTION( exception, "error durring aes 256 cbc encryption init", 
                           ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
    *    * EVP_EncryptUpdate can be called multiple times if necessary
    *       */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    {
        FC_THROW_EXCEPTION( exception, "error durring aes 256 cbc encryption update", 
                           ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
    }
    ciphertext_len = len;

    /* Finalise the encryption. Further ciphertext bytes may be written at
    *    * this stage.
    *       */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) 
    {
        FC_THROW_EXCEPTION( exception, "error durring aes 256 cbc encryption final", 
                           ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
    }
    ciphertext_len += len;

    return ciphertext_len;
}

int aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
              unsigned char *iv, unsigned char *plaintext)
{
    evp_cipher_ctx ctx( EVP_CIPHER_CTX_new() );
    int len = 0;
    int plaintext_len = 0;

    /* Create and initialise the context */
    if(!ctx) 
    {
        FC_THROW_EXCEPTION( exception, "error allocating evp cipher context", 
                           ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
    }

    /* Initialise the decryption operation. IMPORTANT - ensure you use a key
    *    * and IV size appropriate for your cipher
    *       * In this example we are using 256 bit AES (i.e. a 256 bit key). The
    *          * IV size for *most* modes is the same as the block size. For AES this
    *             * is 128 bits */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        FC_THROW_EXCEPTION( exception, "error durring aes 256 cbc decrypt init", 
                           ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
    *    * EVP_DecryptUpdate can be called multiple times if necessary
    *       */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    {
        FC_THROW_EXCEPTION( exception, "error durring aes 256 cbc decrypt update", 
                           ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
    }

    plaintext_len = len;

    /* Finalise the decryption. Further plaintext bytes may be written at
    *    * this stage.
    *       */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) 
    {
        FC_THROW_EXCEPTION( exception, "error durring aes 256 cbc decrypt final", 
                           ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
    }
    plaintext_len += len;

    return plaintext_len;
}



std::vector<char> aes_encrypt( const fc::sha512& key, const std::vector<char>& plain_text  )
{
    std::vector<char> cipher_text(plain_text.size()+16);
    auto cipher_len = aes_encrypt( (unsigned char*)plain_text.data(), plain_text.size(),  
                                 (unsigned char*)&key, ((unsigned char*)&key)+32,
                                 (unsigned char*)cipher_text.data() );
    cipher_text.resize(cipher_len);
    return cipher_text;

}
std::vector<char> aes_decrypt( const fc::sha512& key, const std::vector<char>& cipher_text )
{
    std::vector<char> plain_text( cipher_text.size() );
    auto plain_len = aes_decrypt( (unsigned char*)cipher_text.data(), cipher_text.size(),  
                                 (unsigned char*)&key, ((unsigned char*)&key)+32,
                                 (unsigned char*)plain_text.data() );
    plain_text.resize(plain_len);
    return plain_text;
}

}  // namespace fc
