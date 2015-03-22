# Copyright (c) 2014 Dan Leinir Turthra Jensen <admin@leinir.dk
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# - Try to find the libgit2 library
# Once done this will define
#
#  LIBGIT2_FOUND - System has libgit2
#  LIBGIT2_INCLUDE_DIR - The libgit2 include directory
#  LIBGIT2_LIBRARIES - The libraries needed to use libgit2
#  LIBGIT2_DEFINITIONS - Compiler switches required for using libgit2


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_search_module(PC_LIBGIT2 libgit2)

set(LIBGIT2_DEFINITIONS ${PC_LIBGIT2_CFLAGS_OTHER})

find_path(LIBGIT2_INCLUDE_DIR NAMES git2.h
   HINTS
   ${PC_LIBGIT2_INCLUDEDIR}
   ${PC_LIBGIT2_INCLUDE_DIRS}
)

find_library(LIBGIT2_LIBRARIES NAMES git2
   HINTS
   ${PC_LIBGIT2_LIBDIR}
   ${PC_LIBGIT2_LIBRARY_DIRS}
)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libgit2 DEFAULT_MSG LIBGIT2_LIBRARIES LIBGIT2_INCLUDE_DIR)

mark_as_advanced(LIBGIT2_INCLUDE_DIR LIBGIT2_LIBRARIES)
