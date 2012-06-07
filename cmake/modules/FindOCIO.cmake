# - Find OCIO
# Find the OCIO includes and library
# This module defines
#  OCIO_INCLUDE_DIR, where to find OpenImageIO/version.h
#  OCIO_LIBRARIES, the libraries needed to use OCIO.
#  OCIO_VERSION, The value of OCIO_VERSION defined in oiio.h
#  OCIO_FOUND, If false, do not try to use OCIO.


# Copyright (c) 2008, Adrian Page, <adrian@pagenet.plus.com>
# Copyright (c) 2009, Cyrille Berger, <cberger@cberger.net>
# Copyright (c) 2012, Boudewijn Rempt, <boud@valdyas.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(OCIO_PATH)
    message(STATUS "OCIO path explicitly specified: ${OCIO_PATH}")
endif()

if(OCIO_INCLUDE_PATH)
    message(STATUS "OCIO INCLUDE_PATH explicitly specified: ${OCIO_INCLUDE_PATH}")
endif()

if(OCIO_LIBRARY_PATH)
    message(STATUS "OCIO LIBRARY_PATH explicitly specified: ${OCIO_LIBRARY_PATH}")
endif()

find_path(OCIO_INCLUDE_DIR OpenColorIO.h
        ${OCIO_INCLUDE_PATH}
        ${OCIO_PATH}/include/
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        PATH_SUFFIXES OpenColorIO
        DOC "The directory where OpenColorIO/OpenColorIO.h resides"
)

find_library(OCIO_LIBRARIES OpenColorIO
        PATHS
        ${OCIO_LIBRARY_PATH}
        ${OCIO_PATH}/lib/
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        DOC "The OCIO library"
)

if(OCIO_INCLUDE_DIR AND OCIO_LIBRARIES)
   set(OCIO_FOUND TRUE) 
else(OCIO_INCLUDE_DIR AND OCIO_LIBRARIES)
   set(OCIO_FOUND FALSE)
endif(OCIO_INCLUDE_DIR AND OCIO_LIBRARIES)


if (NOT OCIO_FOUND)
    if(NOT OCIO_FIND_QUIETLY)
        if(OCIO_FIND_REQUIRED)
           message(FATAL_ERROR "Required package OpenColorIO NOT found")
        else(OCIO_FIND_REQUIRED)
           message(STATUS "OpenColorIO NOT found")
        endif(OCIO_FIND_REQUIRED)
    endif(NOT OCIO_FIND_QUIETLY)
endif (NOT OCIO_FOUND)
mark_as_advanced(OCIO_INCLUDE_DIR OCIO_LIBRARIES)
