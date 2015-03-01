 # - Try to find the Fftw3 Library
# Once done this will define
#
#  FFTW3_FOUND - system has fftw3
#  FFTW3_INCLUDE_DIRS - the fftw3 include directories
#  FFTW3_LIBRARIES - the libraries needed to use fftw3
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
if (NOT WIN32)
include(LibFindMacros)
libfind_pkg_check_modules(FFTW3_PKGCONF fftw3>=3.2)

find_path(FFTW3_INCLUDE_DIR
    NAMES fftw3.h
    HINTS ${FFTW3_PKGCONF_INCLUDE_DIRS} ${FFTW3_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES fftw3
)

find_library(FFTW3_LIBRARY
    NAMES fftw3
    HINTS ${FFTW3_PKGCONF_LIBRARY_DIRS} ${FFTW3_PKGCONF_LIBDIR}
)

set(FFTW3_PROCESS_LIBS FFTW3_LIBRARY)
set(FFTW3_PROCESS_INCLUDES FFTW3_INCLUDE_DIR)
libfind_process(FFTW3)

if(FFTW3_FOUND)
    message(STATUS "FFTW Found Version: " ${FFTW_VERSION})
endif()

else()

find_path(FFTW3_INCLUDE_DIR
    NAMES fftw3.h
)

find_path(FFTW3_LIBRARY_DIR
	NAMES libfftw3f-3.lib
)

set (FFTW3_LIBRARIES ${FFTW3_LIBRARY_DIR}/lib/libfftw3f-3.lib 
                     ${FFTW3_LIBRARY_DIR}/lib/libfftw3l-3.lib 
                     ${FFTW3_LIBRARY_DIR}/lib/libfftw3-3.lib )

if(FFTW3_INCLUDE_DIR AND FFTW3_LIBRARY_DIR)
	set (FFTW3_FOUND true)
	message(STATUS "Correctly found FFTW3")
endif()

endif()