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
FIND_PACKAGE(PkgConfig)
PKG_SEARCH_MODULE(PC_LIBQGIT2 libqgit2)

SET(LIBQGIT2_DEFINITIONS ${PC_LIBQGIT2_CFLAGS_OTHER})

FIND_PATH(LIBQGIT2_INCLUDE_DIR NAMES qgit2.h
   HINTS
   ${PC_LIBQGIT2_INCLUDEDIR}
   ${PC_LIBQGIT2_INCLUDE_DIRS}
)

FIND_LIBRARY(LIBQGIT2_LIBRARIES NAMES qgit2
   HINTS
   ${PC_LIBQGIT2_LIBDIR}
   ${PC_LIBQGIT2_LIBRARY_DIRS}
)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libqgit2 DEFAULT_MSG LIBQGIT2_LIBRARIES LIBQGIT2_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBQGIT2_INCLUDE_DIR LIBQGIT2_LIBRARIES)
