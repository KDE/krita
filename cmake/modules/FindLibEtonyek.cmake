# - Try to find LibEtonyek
# Once done this will define
#
#  LIBETONYEK_FOUND       - LibEtonyek is available
#  LIBETONYEK_INCLUDE_DIRS - include directory, e.g. /usr/include
#  LIBETONYEK_LIBRARIES   - the libraries needed to use LibEtonyek
#
# Copyright (C) 2013 Yue Liu <yue.liu@mail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

include(LibFindMacros)
libfind_package(LIBETONYEK LibWpd)
libfind_pkg_check_modules(LIBETONYEK_PKGCONF libetonyek-0.1)

find_path(LIBETONYEK_INCLUDE_DIR
    NAMES libetonyek/libetonyek.h
    HINTS ${LIBETONYEK_PKGCONF_INCLUDE_DIRS} ${LIBETONYEK_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libetonyek-0.1
)

find_library(LIBETONYEK_LIBRARY
    NAMES etonyek etonyek-0.1
    HINTS ${LIBETONYEK_PKGCONF_LIBRARY_DIRS} ${LIBETONYEK_PKGCONF_LIBDIR}
)

set(LIBETONYEK_PROCESS_LIBS LIBETONYEK_LIBRARY LIBWPD_LIBRARIES)
set(LIBETONYEK_PROCESS_INCLUDES LIBETONYEK_INCLUDE_DIR LIBWPD_INCLUDE_DIRS)
libfind_process(LIBETONYEK)
