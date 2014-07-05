# - Try to find the librevenge
# Once done this will define
#
#  LIBREVENGE_FOUND - system has LIBREVENGE
#  LIBREVENGE_INCLUDE_DIRS - the LIBREVENGE include directory
#  LIBREVENGE_LIBRARIES - Link these to use LIBREVENGE
#  LIBREVENGE_DEFINITIONS - Compiler switches required for using LIBREVENGE
#

include(LibFindMacros)
libfind_pkg_check_modules(REVENGE_PKGCONF librevenge-0.0)

find_path(REVENGE_INCLUDE_DIR
    NAMES librevenge/librevenge.h
    HINTS ${REVENGE_PKGCONF_INCLUDE_DIRS} ${REVENGE_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES librevenge-0.0
)

find_path(REVENGE_STREAM_INCLUDE_DIR
    NAMES librevenge-stream/librevenge-stream.h
    HINTS ${REVENGE_STREAM_PKGCONF_INCLUDE_DIRS} ${REVENGE_STREAM_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES librevenge-0.0
)

find_library(REVENGE_LIBRARY
    NAMES revenge librevenge revenge-0.0 librevenge-0.0
    HINTS ${REVENGE_STREAM_PKGCONF_LIBRARY_DIRS} ${REVENGE_STREAM_PKGCONF_LIBDIR}
)

find_library(REVENGE_STREAM_LIBRARY
    NAMES revenge-stream librevenge-stream revenge-stream-0.0 librevenge-stream-0.0
    HINTS ${REVENGE_PKGCONF_LIBRARY_DIRS} ${REVENGE_PKGCONF_LIBDIR}
)

set(LIBREVENGE_PROCESS_LIBS REVENGE_LIBRARY REVENGE_STREAM_LIBRARY)
set(LIBREVENGE_PROCESS_INCLUDES REVENGE_INCLUDE_DIR REVENGE_STREAM_INCLUDE_DIR)
libfind_process(LIBREVENGE)
