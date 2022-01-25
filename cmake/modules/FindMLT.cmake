# Find the MLT includes and libraries
#
# SPDX-FileCopyrightText: 2007-2012 Jean-Baptiste Mardell <jb@kdenlive.org>
# SPDX-FileCopyrightText: 2012 Alberto Villa <avilla@FreeBSD.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.

find_package(PkgConfig QUIET)
pkg_check_modules(PC_MLT mlt++-7)

# Workaround cmake 3.17 issue (dropped .dll extension for library search)
if(WIN32)
    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .dll)
endif()

find_path(MLT_INCLUDE_DIR
  NAMES framework/mlt.h
  HINTS
    ${PC_MLT_INCLUDEDIR}
    ${PC_MLT_PREFIX}/include
  PATHS
    /usr/local/include
    /usr/include
  PATH_SUFFIXES mlt-7
)

find_library(MLT_LIBRARIES
  NAMES mlt-7
  HINTS
    ${PC_MLT_LIBDIR}
    ${PC_MLT_PREFIX}/lib
  PATHS
    /usr/local/lib
    /usr/lib
)

find_path(MLTPP_INCLUDE_DIR
  NAMES mlt++/Mlt.h
  HINTS
    ${PC_MLT_INCLUDEDIR}
    ${PC_MLT_PREFIX}/include
  PATHS
    /usr/local/include
    /usr/include
  PATH_SUFFIXES mlt-7
)

find_library(MLTPP_LIBRARIES
  NAMES mlt++-7
  HINTS
    ${PC_MLT_LIBDIR}
    ${PC_MLT_PREFIX}/lib
  PATHS
    /usr/local/lib
    /usr/lib
)

get_filename_component(MLT_ROOT_DIR ${MLTPP_INCLUDE_DIR} PATH)

if(PC_MLT_VERSION)
  set(MLT_VERSION_STRING ${PC_MLT_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MLT
  REQUIRED_VARS MLT_LIBRARIES MLTPP_LIBRARIES MLT_INCLUDE_DIR MLTPP_INCLUDE_DIR
  VERSION_VAR MLT_VERSION_STRING
)

mark_as_advanced(MLT_INCLUDE_DIR MLT_LIBRARIES MLTPP_INCLUDE_DIR MLTPP_LIBRARIES)
