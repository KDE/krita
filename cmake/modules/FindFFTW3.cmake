 # - Try to find the Fftw3 Library
# Once done this will define
#
#  FFTW3_FOUND - system has Fftw3
#  FFTW3_INCLUDE_DIR - the Fftw3 include directory
#  FFTW3_LIBRARIES 
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (NOT WIN32)
INCLUDE(FindPkgConfig)

pkg_check_modules(FFTW3 fftw3>=3.2)

if (FFTW3_FOUND)
    message(STATUS "FFTW Found Version: " ${FFTW_VERSION})
endif()
	
else (NOT WIN32)

if ( FFTW3_INCLUDE_DIR AND FFTW3_LIBRARIES )
   # in cache already
   SET( FFTW3_FIND_QUIETLY TRUE )
endif ( FFTW3_INCLUDE_DIR AND FFTW3_LIBRARIES )

FIND_PATH( FFTW3_INCLUDE_DIR NAMES fftw3.h PATH_SUFFIXES fftw3
)

FIND_LIBRARY( FFTW3_LIBRARIES NAMES fftw3f-3 )

include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( fftw3 DEFAULT_MSG FFTW3_INCLUDE_DIR FFTW3_LIBRARIES )

endif (NOT WIN32)


