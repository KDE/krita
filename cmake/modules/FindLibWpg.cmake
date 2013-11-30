# - Try to find LibWpg
# Once done this will define
#
#  LIBWPG_FOUND       - libwpg is available
#  LIBWPG_INCLUDE_DIRS - include directory, e.g. /usr/include
#  LIBWPG_LIBRARIES   - the libraries needed to use LibWpg
#  LIBWPG_DEFINITIONS - Compiler switches required for using LibWpg
#
# Copyright (C) 2007 Ariya Hidayat <ariya@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.

include(LibFindMacros)
libfind_package(LIBWPG LibWpd)
libfind_pkg_check_modules(LIBWPG_PKGCONF libwpg-0.2)

find_path(LIBWPG_INCLUDE_DIR
    NAMES libwpg/libwpg.h
    HINTS ${LIBWPG_PKGCONF_INCLUDE_DIRS} ${LIBWPG_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libwpg-0.2
)

find_library(LIBWPG_LIBRARY
    NAMES wpg wpg-0.2
    HINTS ${LIBWPG_PKGCONF_LIBRARY_DIRS} ${LIBWPG_PKGCONF_LIBDIR}
)

set(LIBWPG_PROCESS_LIBS LIBWPG_LIBRARY LIBWPD_LIBRARIES)
set(LIBWPG_PROCESS_INCLUDES LIBWPG_INCLUDE_DIR LIBWPD_INCLUDE_DIRS)
libfind_process(LIBWPG)
