# Try to find LittleCMS.
# Once done this will define:
#
#  LCMS2_FOUND - system has LittleCMS
#  LCMS2_INCLUDE_DIRS - the LittleCMS include directories
#  LCMS_LIBRARIES - the libraries needed to use LittleCMS
#
# Additionally, if the Fast Float plugin is found, it will define:
#
#  LCMS2_FAST_FLOAT_INCLUDE_DIR - the include dir for lcms2_fast_float.h
#  LCMS2_FAST_FLOAT_LIBRARY - the path of the plugin
#
# This module defines both variables and imported targets.
#
# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause
#

include(FindPkgConfig)
pkg_check_modules(LCMS2_PKGCONF LCMS2 QUIET)

find_path(LCMS2_INCLUDE_DIR
    NAMES lcms2.h
    HINTS ${LCMS2_PKGCONF_INCLUDE_DIRS} ${LCMS2_PKGCONF_INCLUDEDIR}
    DOC "LittleCMS 2 headers include folder"
    PATH_SUFFIXES lcms2 liblcms2
)

find_path(LCMS2_FAST_FLOAT_INCLUDE_DIR
    NAMES lcms2_fast_float.h
    HINTS ${LCMS2_PKGCONF_INCLUDE_DIRS} ${LCMS2_PKGCONF_INCLUDEDIR} ${LCMS2_INCLUDE_DIR}
    DOC "LittleCMS 2 Fast Float Plugin headers include folder"
    PATH_SUFFIXES lcms2 liblcms2
)

find_library(LCMS2_LIBRARY
    NAMES lcms2 liblcms2 lcms-2 liblcms-2
    HINTS ${LCMS2_PKGCONF_LIBRARY_DIRS} ${LCMS2_PKGCONF_LIBDIR}
    DOC "LittleCMS 2 library"
    PATH_SUFFIXES lcms2
)

find_library(LCMS2_FAST_FLOAT_LIBRARY
    NAMES lcms2_fast_float liblcms2_fast_float lcms-2_fast_float liblcms-2_fast_float
    HINTS ${LCMS2_PKGCONF_LIBRARY_DIRS} ${LCMS2_PKGCONF_LIBDIR}
    DOC "LittleCMS 2 Fast Float Plugin library"
    PATH_SUFFIXES lcms2
)

if(LCMS2_INCLUDE_DIR)
    set(lcms2_config_file "${LCMS2_INCLUDE_DIR}/lcms2.h")
    if(EXISTS ${lcms2_config_file})
        file(STRINGS
            ${lcms2_config_file}
            TMP
            REGEX "#define LCMS_VERSION.*$")
        string(REGEX MATCHALL "[0-9.]+" LCMS2_VER_STRING ${TMP})
        string(SUBSTRING ${LCMS2_VER_STRING} 0 1 LCMS2_MAJOR_VERSION)
        string(SUBSTRING ${LCMS2_VER_STRING} 1 2 LCMS2_MINOR_VERSION)
        string(SUBSTRING ${LCMS2_VER_STRING} 3 -1 LCMS2_PATCH_VERSION)
        set(LCMS2_VERSION "${LCMS2_MAJOR_VERSION}.${LCMS2_MINOR_VERSION}.${LCMS2_PATCH_VERSION}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LCMS2
  FOUND_VAR LCMS2_FOUND
  REQUIRED_VARS
    LCMS2_LIBRARY
    LCMS2_INCLUDE_DIR
  VERSION_VAR LCMS2_VERSION
)

if(LCMS2_FOUND)
    set(LCMS2_LIBRARIES ${LCMS2_LIBRARY})
    set(LCMS2_INCLUDE_DIRS ${LCMS2_INCLUDE_DIR})
    if(LCMS2_FAST_FLOAT_LIBRARY)
        list(APPEND LCMS2_LIBRARIES ${LCMS2_FAST_FLOAT_LIBRARY})
    endif()
    if(LCMS2_FAST_FLOAT_INCLUDE_DIR)
      list(APPEND LCMS2_INCLUDE_DIRS ${LCMS2_FAST_FLOAT_INCLUDE_DIR})
      list(REMOVE_DUPLICATES LCMS2_INCLUDE_DIRS)
    endif()
    if(LCMS2_PKG_CONF_CFLAGS_OTHER)
        set(LCMS2_DEFINITIONS ${LCMS2_PKG_CONF_CFLAGS_OTHER})
    endif()
endif()

if(LCMS2_FOUND AND NOT TARGET LCMS2::LCMSFastFloat AND LCMS2_FAST_FLOAT_LIBRARY)
  add_library(LCMS2::LCMSFastFloat UNKNOWN IMPORTED)
  set_target_properties(LCMS2::LCMSFastFloat PROPERTIES
    IMPORTED_LOCATION "${LCMS2_FAST_FLOAT_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${LCMS2_PKG_CONF_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${LCMS2_FAST_FLOAT_INCLUDE_DIR}"
  )
endif()

if(LCMS2_FOUND AND NOT TARGET LCMS2::LCMS)
  add_library(LCMS2::LCMS UNKNOWN IMPORTED)
  set_target_properties(LCMS2::LCMS PROPERTIES
    IMPORTED_LOCATION "${LCMS2_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${LCMS2_PKG_CONF_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${LCMS2_INCLUDE_DIR}"
  )
endif()
