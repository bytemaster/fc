include_directories(include)
include_directories( ${OPENSSL_INCLUDE_DIR} )
SET( sources 
    src/agent.c
    src/channel.c
    src/comp.c
    src/crypt.c
    src/global.c
    src/hostkey.c
    src/keepalive.c
    src/kex.c
    src/knownhost.c
    src/mac.c
    src/misc.c
    src/openssl.c
    src/packet.c
    src/publickey.c
    src/scp.c
    src/session.c
    src/sftp.c
    src/transport.c
    src/userauth.c
    src/version.c
)

SETUP_LIBRARY( ssh2 SOURCES ${sources} LIBRARIES  ${libraries} LIBRARY_TYPE STATIC )
