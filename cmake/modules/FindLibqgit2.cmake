# Copyright (c) 2014 Dan Leinir Turthra Jensen <admin@leinir.dk
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# - Try to find the libqgit2 library
# Once done this will define
#
#  LIBQGIT2_FOUND - System has libqgit2
#  LIBQGIT2_INCLUDE_DIR - The libqgit2 include directory
#  LIBQGIT2_LIBRARIES - The libraries needed to use libqgit2
#  LIBQGIT2_DEFINITIONS - Compiler switches required for using libqgit2


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_search_module(PC_LIBQGIT2 libqgit2)

set(LIBQGIT2_DEFINITIONS ${PC_LIBQGIT2_CFLAGS_OTHER})

find_path(LIBQGIT2_INCLUDE_DIR NAMES qgit2.h
   HINTS
   ${PC_LIBQGIT2_INCLUDEDIR}
   ${PC_LIBQGIT2_INCLUDE_DIRS}
)

find_library(LIBQGIT2_LIBRARIES NAMES qgit2
   HINTS
   ${PC_LIBQGIT2_LIBDIR}
   ${PC_LIBQGIT2_LIBRARY_DIRS}
)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libqgit2 DEFAULT_MSG LIBQGIT2_LIBRARIES LIBQGIT2_INCLUDE_DIR)

mark_as_advanced(LIBQGIT2_INCLUDE_DIR LIBQGIT2_LIBRARIES)
