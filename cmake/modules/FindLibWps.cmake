# - Try to find LibWps
# Once done this will define
#
#  LIBWPS_FOUND       - libwps is available
#  LIBWPS_INCLUDE_DIRS - include directory, e.g. /usr/include
#  LIBWPS_LIBRARIES   - the libraries needed to use LibWps
#
# Copyright (C) 2013 Yue Liu <yue.liu@mail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

include(LibFindMacros)
libfind_package(LIBWPS LibWpd)
libfind_pkg_check_modules(LIBWPS_PKGCONF libwps-0.2)

find_path(LIBWPS_INCLUDE_DIR
    NAMES libwps/libwps.h
    HINTS ${LIBWPS_PKGCONF_INCLUDE_DIRS} ${LIBWPS_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libwps-0.2
)

find_library(LIBWPS_LIBRARY
    NAMES wps wps-0.2
    HINTS ${LIBWPS_PKGCONF_LIBRARY_DIRS} ${LIBWPS_PKGCONF_LIBDIR}
)

set(LIBWPS_PROCESS_LIBS LIBWPS_LIBRARY LIBWPD_LIBRARIES)
set(LIBWPS_PROCESS_INCLUDES LIBWPS_INCLUDE_DIR LIBWPD_INCLUDE_DIRS)
libfind_process(LIBWPS)
