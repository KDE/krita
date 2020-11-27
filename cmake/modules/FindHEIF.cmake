# - Try to find the libheif library
# Once done this will define
#
#  HEIF_FOUND - system has heif
#  HEIF_INCLUDE_DIRS - the heif include directories
#  HEIF_LIBRARIES - the libraries needed to use heif
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(LibFindMacros)
libfind_pkg_check_modules(HEIF_PKGCONF libheif)

find_path(HEIF_INCLUDE_DIR
    NAMES libheif/heif.h
    HINTS ${HEIF_PKGCONF_INCLUDE_DIRS} ${HEIF_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES heif
)

find_library(HEIF_LIBRARY
    NAMES libheif heif
    HINTS ${HEIF_PKGCONF_LIBRARY_DIRS} ${HEIF_PKGCONF_LIBDIR}
    DOC "Libraries to link against for HEIF Support"
)

set(HEIF_PROCESS_LIBS HEIF_LIBRARY)
set(HEIF_PROCESS_INCLUDES HEIF_INCLUDE_DIR)
libfind_process(HEIF)

if(HEIF_INCLUDE_DIR)
  set(heif_config_file "${HEIF_INCLUDE_DIR}/libheif/heif_version.h")
  if(EXISTS ${heif_config_file})
      file(STRINGS
           ${heif_config_file}
           TMP
           REGEX "#define LIBHEIF_VERSION.*$")
      string(REGEX MATCHALL "[0-9.]+" LIBHEIF_VERSION ${TMP})
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HEIF
    REQUIRED_VARS
        HEIF_INCLUDE_DIR
        HEIF_LIBRARY
    VERSION_VAR
        LIBHEIF_VERSION
)
