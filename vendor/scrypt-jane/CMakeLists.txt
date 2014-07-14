add_definitions( -DSCRYPT_SALSA )
add_definitions( -DSCRYPT_SHA256 )

set( scrypt_sources
     scrypt-jane.c
   )

add_library( scrypt ${scrypt_sources} )
