PROJECT( fc )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8.0 )

SET( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )
SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )

INCLUDE( VersionMacros )
INCLUDE( SetupTargetMacros )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cyoencode-1.0.2/src )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/vendor/boost_1.51/include )

SET( DEFAULT_HEADER_INSTALL_DIR include/\${target} )
SET( DEFAULT_LIBRARY_INSTALL_DIR lib/ )
SET( DEFAULT_EXECUTABLE_INSTALL_DIR bin/ )
SET( CMAKE_DEBUG_POSTFIX _debug )
SET( BUILD_SHARED_LIBS NO )
SET(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost 1.51 COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context ) 

if( WIN32 )
   set( RPCRT4 Rpcrt4 )
endif( WIN32 )

LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

IF( WIN32 )
    ADD_DEFINITIONS( -DWIN32 )
	ADD_DEFINITIONS( -DBOOST_CONTEXT_NO_LIB )
	ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
	ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
	ADD_DEFINITIONS( -DBOOST_ALL_NO_LIB -DBOOST_THREAD_BUILD_LIB)
	ADD_DEFINITIONS( -DWIN32)
	# Activate C++ exception handling
	IF (NOT CMAKE_CXX_FLAGS MATCHES "/EHsc")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")    
	ENDIF()
ELSE(WIN32)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-unused-local-typedefs -fmax-errors=3" )
ENDIF(WIN32)

if( UNIX ) 
  if( NOT APPLE )  
    set(rt_library rt )
	set(pthread_library pthread) 
  endif() 
endif()

option( UNITY_BUILD OFF )

FIND_PACKAGE( OpenSSL )
include_directories( ${Boost_INCLUDE_DIR} )
include_directories( ${OPENSSL_INCLUDE_DIR} )

SET( ALL_OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES} ${SSL_EAY_RELEASE} ${LIB_EAY_RELEASE})

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
     src/io/iostream.cpp
     src/io/datastream.cpp
     src/io/buffered_iostream.cpp
     src/io/fstream.cpp
     src/io/sstream.cpp
     src/io/json.cpp
     src/io/varint.cpp
     src/filesystem.cpp
     src/interprocess/process.cpp
     src/interprocess/file_mapping.cpp
     src/interprocess/mmap_struct.cpp
     src/rpc/json_connection.cpp
     src/log/log_message.cpp 
     src/log/logger.cpp
     src/log/appender.cpp
     src/log/console_appender.cpp
     src/log/file_appender.cpp
     src/log/logger_config.cpp
     src/crypto/crc.cpp
     src/crypto/city.cpp
     src/crypto/base32.cpp
     src/crypto/base36.cpp
     src/crypto/base58.cpp
     src/crypto/base64.cpp
     src/crypto/bigint.cpp
     src/crypto/hex.cpp
     src/crypto/sha1.cpp
     src/crypto/sha256.cpp
     src/crypto/sha224.cpp
     src/crypto/sha512.cpp
     src/crypto/dh.cpp
     src/crypto/blowfish.cpp
     src/crypto/elliptic.cpp
     src/network/tcp_socket.cpp
     src/network/udp_socket.cpp
     src/network/http/http_connection.cpp
     src/network/http/http_server.cpp
     src/network/ip.cpp
     src/network/resolve.cpp
     src/network/url.cpp
     vendor/cyoencode-1.0.2/src/CyoDecode.c
     vendor/cyoencode-1.0.2/src/CyoEncode.c
#     src/ssh/client.cpp
#     src/ssh/process.cpp
     )

IF( UNIX )
    IF( ENABLE_ARM )
      SET_PROPERTY( SOURCE 
             vendor/boost_1.51/libs/context/asm/fcontext_arm_aapcs_elf_gas.S 
             PROPERTY LANGUAGE C)
      SET( boost_context_sources vendor/boost_1.51/libs/context/protected_stack_posix.cpp 
              vendor/boost_1.51/libs/context/stack_helper_posix.cpp 
              vendor/boost_1.51/libs/context/asm/fcontext_arm_aapcs_elf_gas.S 
              )
      ELSE()

      IF( APPLE )
           SET_PROPERTY( SOURCE 
                                         vendor/boost_1.51/libs/context/asm/fcontext_x86_64_sysv_macho_gas.S 
                                                         PROPERTY LANGUAGE C)
                   SET( boost_context_sources vendor/boost_1.51/libs/context/stack_utils_posix.cpp
                         vendor/boost_1.51/libs/context/stack_allocator_posix.cpp
                         vendor/boost_1.51/libs/context/fcontext.cpp
                         vendor/boost_1.51/libs/context/asm/fcontext_x86_64_sysv_macho_gas.S 
                         )
      ELSE (APPLE)
         IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
             SET_PROPERTY( SOURCE 
                           vendor/boost_1.51/libs/context/asm/fcontext_i386_sysv_elf_gas.S
                           PROPERTY LANGUAGE C)
                   SET( fcontext_asm vendor/boost_1.51/libs/context/asm/fcontext_i386_sysv_elf_gas.S )
         ELSE( )
          SET( fcontext_asm vendor/boost_1.51/libs/context/asm/fcontext_x86_64_sysv_elf_gas.S )
          SET_PROPERTY( SOURCE 
                        vendor/boost_1.51/libs/context/asm/fcontext_x86_64_sysv_elf_gas.S
                        PROPERTY LANGUAGE C)
              ENDIF( )
         
         SET( boost_context_sources vendor/boost_1.51/libs/context/stack_utils_posix.cpp
                                  vendor/boost_1.51/libs/context/stack_allocator_posix.cpp
                                  vendor/boost_1.51/libs/context/fcontext.cpp
                                  ${fcontext_asm}
                                  )
      ENDIF(APPLE)

    ENDIF()
ENDIF( UNIX )

IF( WIN32 )
  ENABLE_LANGUAGE(ASM_MASM) 
#  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa" )

  SET( boost_context_sources vendor/boost_1.51/libs/context/fcontext.cpp
             vendor/boost_1.51/libs/context/seh.cpp
             vendor/boost_1.51/libs/context/stack_allocator_windows.cpp
             vendor/boost_1.51/libs/context/stack_utils_windows.cpp 
             vendor/boost_1.51/libs/context/seh.cpp 
     )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    SET( boost_context_sources ${boost_context_sources} 
             vendor/boost_1.51/libs/context/asm/fcontext_x86_64_ms_pe_masm.asm )
  ELSE() # 32bit
    SET( boost_context_sources ${boost_context_sources} 
             vendor/boost_1.51/libs/context/asm/fcontext_i386_ms_pe_masm.asm )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO" )
    message(STATUS "")
    message(STATUS "Note for Visual Studio 2012 projects:")
    message(STATUS "  Each time CMake regenerates your projects, you will need to:")
    message(STATUS "  Right-click on the 'fc' project and select 'Build Customizations', and")
    message(STATUS "    check the 'masm' customization.")
    message(STATUS "  then, go to the 'Properties' for the fcontext*.asm file, and change its ")
    message(STATUS "  item type from 'Does not participate in build' to 'Microsoft Macro Assembler'")
  ENDIF()

ENDIF(WIN32)

set( sources
  ${fc_sources}
  ${boost_context_sources}
)

setup_library( fc SOURCES ${sources} LIBRARY_TYPE STATIC )


