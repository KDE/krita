 # - Try to find the Fftw3 Library
# Once done this will define
#
#  FFTW3_FOUND - system has fftw3
#  FFTW3_INCLUDE_DIRS - the fftw3 include directories
#  FFTW3_LIBRARIES - the libraries needed to use fftw3
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

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
