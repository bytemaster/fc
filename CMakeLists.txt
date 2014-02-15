#
# Defines fc library target.

PROJECT( fc )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

MESSAGE(STATUS "Configuring project fc located in: ${CMAKE_CURRENT_SOURCE_DIR}")
SET( CMAKE_AUTOMOC OFF )

# Setup module path to make visible used CMake extensions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GitVersionGen")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

INCLUDE( VersionMacros )
INCLUDE( SetupTargetMacros )
INCLUDE(GetGitRevisionDescription)

get_git_head_revision(GIT_REFSPEC GIT_SHA3)

SET( DEFAULT_HEADER_INSTALL_DIR include/\${target} )
SET( DEFAULT_LIBRARY_INSTALL_DIR lib/ )
SET( DEFAULT_EXECUTABLE_INSTALL_DIR bin/ )
SET( CMAKE_DEBUG_POSTFIX _debug )
SET( BUILD_SHARED_LIBS NO )

SET (ORIGINAL_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

IF( WIN32 )
  MESSAGE(STATUS "Configuring fc to build on Win32")

  #You need to set OPENSSL_ROOT environment variable for your system on WIN32
  message(STATUS "Setting up OpenSSL root and include vars on Win32 platform")
  set( OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT} )

  set( RPCRT4 Rpcrt4 )

  #boost
  SET(BOOST_ROOT $ENV{BOOST_ROOT})
  set(Boost_USE_DEBUG_PYTHON ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_LIBS OFF)
  set(BOOST_ALL_DYN_LINK ON) # force dynamic linking for all libraries
  
  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context)
  # For Boost 1.53 on windows, coroutine was not in BOOST_LIBRARYDIR and do not need it to build,  but if boost versin >= 1.54, find coroutine otherwise will cause link errors
  IF(NOT "${Boost_VERSION}" MATCHES "1.53(.*)")
    SET(BOOST_LIBRARIES_TEMP ${Boost_LIBRARIES})
    FIND_PACKAGE(Boost 1.54 REQUIRED COMPONENTS coroutine)
    SET(Boost_LIBRARIES ${BOOST_LIBRARIES_TEMP} ${Boost_LIBRARIES})
  ENDIF()

ELSE(WIN32)
  MESSAGE(STATUS "Configuring fc to build on Unix/Apple")

  SET(Boost_USE_STATIC_LIBS ON)
  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context coroutine) 

  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")

  include_directories( ${Boost_INCLUDE_DIR} )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/vendor/salsa20 )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/vendor/easylzma/src )
  IF(NOT APPLE ) # then unix
    # Unix build:
    SET(SALSA_SRC vendor/salsa20/salsa20.s)
    SET(rt_library rt )
    SET(pthread_library pthread)
  ENDIF(NOT APPLE)
ENDIF(WIN32)

FIND_PACKAGE( OpenSSL )

SET (CMAKE_FIND_LIBRARY_SUFFIXES ${ORIGINAL_LIB_SUFFIXES})

option( UNITY_BUILD OFF )

set( fc_sources
     src/uint128.cpp
     src/variant.cpp
     src/exception.cpp
     src/variant_object.cpp
     src/thread/thread.cpp
     src/thread/future.cpp
     src/thread/task.cpp
     src/thread/spin_lock.cpp 
     src/thread/spin_yield_lock.cpp 
     src/thread/mutex.cpp
     src/asio.cpp
     src/string.cpp
     src/shared_ptr.cpp
     src/time.cpp
     src/utf8.cpp
     src/io/iostream.cpp
     src/io/datastream.cpp
     src/io/buffered_iostream.cpp
     src/io/fstream.cpp
     src/io/sstream.cpp
     src/io/json.cpp
     src/io/varint.cpp
     src/filesystem.cpp
     src/interprocess/process.cpp
     src/interprocess/signals.cpp
     src/interprocess/file_mapping.cpp
     src/interprocess/mmap_struct.cpp
     src/rpc/json_connection.cpp
     src/log/log_message.cpp 
     src/log/logger.cpp
     src/log/appender.cpp
     src/log/console_appender.cpp
     src/log/file_appender.cpp
     src/log/logger_config.cpp
     src/crypto/openssl.cpp
     src/crypto/aes.cpp
     src/crypto/crc.cpp
     src/crypto/city.cpp
     src/crypto/base32.cpp
     src/crypto/base36.cpp
     src/crypto/base58.cpp
     src/crypto/base64.cpp
     src/crypto/bigint.cpp
     src/crypto/hex.cpp
     src/crypto/sha1.cpp
     src/crypto/ripemd160.cpp
     src/crypto/sha256.cpp
     src/crypto/sha224.cpp
     src/crypto/sha512.cpp
     src/crypto/dh.cpp
     src/crypto/blowfish.cpp
     src/crypto/elliptic.cpp
     src/crypto/salsa20.cpp
     src/network/tcp_socket.cpp
     src/network/udp_socket.cpp
     src/network/http/http_connection.cpp
     src/network/http/http_server.cpp
     src/network/ip.cpp
     src/network/resolve.cpp
     src/network/url.cpp
     src/compress/smaz.cpp
     src/compress/lzma.cpp
     vendor/cyoencode-1.0.2/src/CyoDecode.c
     vendor/cyoencode-1.0.2/src/CyoEncode.c
#     vendor/salsa20/ecrypt.c
     ${SALSA_SRC}
     )

     SET_PROPERTY( SOURCE 
                   vendor/salsa20/salsa20.s
                   PROPERTY LANGUAGE C)

file(GLOB_RECURSE fc_headers ${CMAKE_CURRENT_SOURCE_DIR} 
  *.hpp *.h)

set( sources
  ${fc_sources}
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA3.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA3.cpp" @ONLY)
list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/GitSHA3.cpp" GitSHA3.h)
list(APPEND sources ${fc_headers})

add_subdirectory( vendor/easylzma )

setup_library( fc SOURCES ${sources} LIBRARY_TYPE STATIC )

IF(WIN32)
  target_compile_definitions(fc PUBLIC WIN32 NOMINMAX _WIN32_WINNT=0x0501 _CRT_SECURE_NO_WARNINGS
    _SCL_SERCURE_NO_WARNINGS 
    # Needed to disable MSVC autolinking feature (#pragma comment)
    BOOST_ALL_NO_LIB
  )
  # Activate C++ exception handling inc. SEH to catch GPFs
  target_compile_options(fc PUBLIC /EHa)
ELSE()
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall" )

  IF(APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -stdlib=libc++  -Wall" )
  ELSE()
    target_compile_options(fc PUBLIC -std=c++11 -Wall -fnon-call-exceptions -Wno-unused-local-typedefs -fmax-errors=3)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-local-typedefs -fmax-errors=3 ")
  ENDIF()
ENDIF()
 
target_include_directories(fc
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}

  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cyoencode-1.0.2/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/boost_1.51/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/salsa20
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/easylzma/src
  )

target_link_libraries( fc easylzma_static ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )

#add_executable( test_compress tests/compress.cpp )
#target_link_libraries( test_compress fc )
#add_executable( test_aes tests/aes_test.cpp )
#target_link_libraries( test_aes fc ${rt_library} ${pthread_library} )
#add_executable( test_sleep tests/sleep.cpp )
#target_link_libraries( test_sleep fc )

MESSAGE(STATUS "Finished fc module configuration...")

