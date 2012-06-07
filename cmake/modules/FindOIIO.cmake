# - Find OIIO
# Find the OIIO includes and library
# This module defines
#  OIIO_INCLUDE_DIR, where to find OpenImageIO/version.h
#  OIIO_LIBRARIES, the libraries needed to use OIIO.
#  OIIO_VERSION, The value of OIIO_VERSION defined in oiio.h
#  OIIO_FOUND, If false, do not try to use OIIO.


# Copyright (c) 2008, Adrian Page, <adrian@pagenet.plus.com>
# Copyright (c) 2009, Cyrille Berger, <cberger@cberger.net>
# Copyright (c) 2012, Boudewijn Rempt, <boud@valdyas.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(OIIO_PATH)
    message(STATUS "OIIO path explicitly specified: ${OIIO_PATH}")
endif()

if(OIIO_INCLUDE_PATH)
    message(STATUS "OIIO INCLUDE_PATH explicitly specified: ${OIIO_INCLUDE_PATH}")
endif()

if(OIIO_LIBRARY_PATH)
    message(STATUS "OIIO LIBRARY_PATH explicitly specified: ${OIIO_LIBRARY_PATH}")
endif()

find_path(OIIO_INCLUDE_DIR OpenImageIO/version.h
        ${OIIO_INCLUDE_PATH}
        ${OIIO_PATH}/include/
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        DOC "The directory where OpenImageIO/version.h resides")
)

find_library(OIIO_LIBRARIES
        NAMES OIIO OpenImageIO
        PATHS
        ${OIIO_LIBRARY_PATH}
        ${OIIO_PATH}/lib/
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        DOC "The OIIO library")
)

if(OIIO_INCLUDE_DIR AND OIIO_LIBRARIES)
   set(OIIO_FOUND TRUE)
else(OIIO_INCLUDE_DIR AND OIIO_LIBRARIES)
   set(OIIO_FOUND FALSE)
endif(OIIO_INCLUDE_DIR AND OIIO_LIBRARIES)

mark_as_advanced(OIIO_INCLUDE_DIR OIIO_LIBRARIES)
