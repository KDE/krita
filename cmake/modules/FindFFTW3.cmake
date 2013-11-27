 # - Try to find the Fftw3 Library
# Once done this will define
#
#  FFTW3_FOUND - system has fftw3
#  FFTW3_INCLUDE_DIRS - the fftw3 include directories
#  FFTW3_LIBRARIES - the libraries needed to use fftw3
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if(NOT WIN32)
include(FindPkgConfig)
pkg_check_modules(FFTW3 QUIET fftw3>=3.2)
endif()

find_path(FFTW3_INCLUDE_DIR
    NAMES fftw3.h
    HINTS ${PC_FFTW3_INCLUDEDIR} ${PC_FFTW3_INCLUDE_DIRS}
    PATH_SUFFIXES fftw3)

find_library(FFTW3_LIBRARY
    NAMES fftw3
    HINTS ${PC_FFTW3_LIBDIR} ${PC_FFTW3_LIBRARY_DIRS})

set(FFTW3_LIBRARIES ${FFTW3_LIBRARY})
set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(fftw3 DEFAULT_MSG FFTW3_LIBRARY FFTW3_INCLUDE_DIR)

mark_as_advanced(FFTW3_INCLUDE_DIR FFTW3_LIBRARY)

if(FFTW3_FOUND)
    message(STATUS "FFTW Found Version: " ${FFTW_VERSION})
endif()
