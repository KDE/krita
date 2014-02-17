# - Try to find LibOdfGen
# Once done this will define
#
#  LIBODFGEN_FOUND       - libodfgen is available
#  LIBODFGEN_INCLUDE_DIRS - include directory, e.g. /usr/include
#  LIBODFGEN_LIBRARIES   - the libraries needed to use LibOdfGen
#
# Copyright (C) 2013 Yue Liu <yue.liu@mail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

include(LibFindMacros)
libfind_pkg_check_modules(LIBODFGEN_PKGCONF libodfgen-0.0)

find_path(LIBODFGEN_INCLUDE_DIR
    NAMES libodfgen/libodfgen.hxx
    HINTS ${LIBODFGEN_PKGCONF_INCLUDE_DIRS} ${LIBODFGEN_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libodfgen-0.0
)

find_library(LIBODFGEN_LIBRARY
    NAMES odfgen-0.0
    HINTS ${LIBODFGEN_PKGCONF_LIBRARY_DIRS} ${LIBODFGEN_PKGCONF_LIBDIR}
)

set(LIBODFGEN_PROCESS_LIBS LIBODFGEN_LIBRARY)
set(LIBODFGEN_PROCESS_INCLUDES LIBODFGEN_INCLUDE_DIR)
libfind_process(LIBODFGEN)
