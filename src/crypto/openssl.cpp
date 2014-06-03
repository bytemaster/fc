#include <fc/crypto/openssl.hpp>

#include <fc/filesystem.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

namespace  fc 
{
    struct openssl_scope
    {
      static path _configurationFilePath;
       openssl_scope()
       {
          ERR_load_crypto_strings(); 
          OpenSSL_add_all_algorithms();
          const boost::filesystem::path& boostPath = _configurationFilePath;
          OPENSSL_config(boostPath.empty() ? nullptr : _configurationFilePath.to_native_ansi_path().c_str());
       }

       ~openssl_scope()
       {
          EVP_cleanup();
          ERR_free_strings();
       }
    };

    path openssl_scope::_configurationFilePath;

    void store_configuration_path(const path& filePath)
      {
      openssl_scope::_configurationFilePath = filePath;
      }
   
    int init_openssl()
    {
        static openssl_scope ossl;
        return 0;
    }
}
