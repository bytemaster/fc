PROJECT( fc )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8.0 )

SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )

INCLUDE( VersionMacros )
INCLUDE( SetupTargetMacros )

SET( DEFAULT_HEADER_INSTALL_DIR include/\${target} )
SET( DEFAULT_LIBRARY_INSTALL_DIR lib/ )
SET( DEFAULT_EXECUTABLE_INSTALL_DIR bin/ )
SET( CMAKE_DEBUG_POSTFIX _debug )
#SET( BUILD_SHARED_LIBS NO )

SET(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost 1.51 COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context ) 



INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} )
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

IF( WIN32 )
	ADD_DEFINITIONS( -DBOOST_CONTEXT_NO_LIB )
	ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
	ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
ELSE(WIN32)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
ENDIF(WIN32)

if( UNIX ) 
  if( NOT APPLE )  
    set(rt_library rt )
	set(pthread_library pthread) 
  endif() 
endif()

option( UNITY_BUILD OFF )

FIND_PACKAGE( OpenSSL )

include_directories( vendor/boost/process/include )
include_directories( ${Boost_INCLUDE_DIR} )
include_directories( ${OPENSSL_INCLUDE_DIR} )
include_directories( include )

set( sources 
  src/ssh.cpp
  src/process.cpp
  src/http_connection.cpp
  src/http_server.cpp
  src/json_rpc_connection.cpp
  src/json_rpc_stream_connection.cpp
  src/json_rpc_tcp_connection.cpp
  src/json_rpc_tcp_server.cpp
  src/json_rpc_error_object.cpp
  src/value.cpp
  src/lexical_cast.cpp
  src/spin_lock.cpp 
  src/spin_yield_lock.cpp 
  src/task.cpp 
  src/future.cpp 
  src/shared_ptr.cpp 
  src/string.cpp 
  src/json.cpp 
  src/log.cpp 
  src/time.cpp 
  src/iostream.cpp
  src/fstream.cpp
  src/sstream.cpp
  src/exception.cpp
  src/thread.cpp
  src/hex.cpp
  src/sha1.cpp
  src/filesystem.cpp
  src/ip.cpp
  src/bigint.cpp
  src/mutex.cpp
  src/pke.cpp
  src/base64.cpp
  src/base58.cpp
  src/blowfish.cpp
  src/dh.cpp
  src/udp_socket.cpp
  src/tcp_socket.cpp
  src/asio.cpp
  src/super_fast_hash.cpp
  src/file_mapping.cpp
#  src/program_options.cpp
)

#add_subdirectory(vendor/libssh2-1.4.2)

setup_library( fc SOURCES ${sources} )

setup_executable( json_rpc_test SOURCES tests/json_rpc_test.cpp LIBRARIES fc ${pthread_library} ${rt_library} ${Boost_THREAD_LIBRARY} ${Boost_CONTEXT_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${rt_library}  ${Boost_DATE_TIME_LIBRARY})
setup_executable( ssh_test SOURCES tests/ssh.cpp LIBRARIES fc ${pthread_library} ${rt_library} ${Boost_THREAD_LIBRARY} ${Boost_CONTEXT_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${rt_library} ssh2 ${OPENSSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY} ${ZLIB_LIBRARY} ${ALL_OPENSSL_LIBRARIES} ${Boost_DATE_TIME_LIBRARY})

#add_executable( test_vec tests/vector_test.cpp )
#target_link_libraries( test_vec fc ${Boost_SYSTEM_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_CONTEXT_LIBRARY} )

#add_executable( unit_tests tests/unit.cpp )
#target_link_libraries( unit_tests fc ${Boost_CHRONO_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_CONTEXT_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )
