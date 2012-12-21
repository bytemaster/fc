INCLUDE_DIRECTORIES(include)


IF(WIN32)
  INCLUDE_DIRECTORIES(src/os/win32)
  SET( os_sources src/os/win32/peb.c src/os/win32/wmi.cpp src/os/win32/win32_sigar.c )
ELSE(WIN32)
  IF( APPLE )
  ADD_DEFINITIONS( -DDARWIN_HAS_LIBPROC_H )
  INCLUDE_DIRECTORIES(src/os/darwin)
  SET( os_sources  src/os/darwin/darwin_sigar.c )
  ELSE( APPLE )
  INCLUDE_DIRECTORIES(src/os/linux)
  SET( os_sources  src/os/linux/linux_sigar.c )
  ENDIF( APPLE )

ENDIF(WIN32)


SET( sources 
   ${os_sources}
   src/sigar.c
   src/sigar_cache.c
   src/sigar_fileinfo.c
   src/sigar_format.c
   src/sigar_getline.c
   src/sigar_ptql.c
   src/sigar_signal.c
   src/sigar_util.c
   src/sigar_version_autoconf.c
)

SETUP_LIBRARY( sigar SOURCES ${sources} LIBRARIES  ${libraries} LIBRARY_TYPE STATIC )
