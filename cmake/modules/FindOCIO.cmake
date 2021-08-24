# - Find OCIO
# Find the OCIO includes and library
# This module defines
#  OCIO_INCLUDE_DIR, where to find OpenImageIO/version.h
#  OCIO_LIBRARIES, the libraries needed to use OCIO.
#  OCIO_VERSION, The value of OCIO_VERSION defined in oiio.h
#  OCIO_FOUND, If false, do not try to use OCIO.


# SPDX-FileCopyrightText: 2008 Adrian Page <adrian@pagenet.plus.com>
# SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
# SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#
include(UsePkgConfig)

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
        PATHS
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
    file(STRINGS ${OCIO_INCLUDE_DIR}/OpenColorABI.h ocio_version REGEX "^#define OCIO_VERSION[ \t\r\n]*\"(.+)\"$")
    string(REGEX REPLACE "^#define OCIO_VERSION[ \t\r\n]*\"(.+)\"$" "\\1" ocio_version "${ocio_version}")
    SET (OCIO_VERSION ${ocio_version})
else()
   set(OCIO_FOUND FALSE)
endif()

if ( OCIO_VERSION STREQUAL ${OCIO_FIND_VERSION})
    message(STATUS "OCIO version ${OCIO_VERSION} found")
else()
    set(OCIO_FOUND FALSE)
endif()

if (NOT OCIO_FOUND)
    if(NOT OCIO_FIND_QUIETLY)
        if(OCIO_FIND_REQUIRED)
           message(FATAL_ERROR "Required package OpenColorIO NOT found")
        else()
           message(STATUS "OpenColorIO version ${OCIO_FIND_VERSION} NOT found")
        endif()
    endif()
endif ()
mark_as_advanced(OCIO_INCLUDE_DIR OCIO_LIBRARIES)
