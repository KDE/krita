# - Try to find the libwpd (WordPerfect library)
# Once done this will define
#
#  LIBWPD_FOUND - system has LIBWPD
#  LIBWPD_INCLUDE_DIRS - the LIBWPD include directory
#  LIBWPD_LIBRARIES - Link these to use LIBWPD
#  LIBWPD_DEFINITIONS - Compiler switches required for using LIBWPD
#

include(LibFindMacros)
libfind_pkg_check_modules(WPD_PKGCONF libwpd-0.9)
libfind_pkg_check_modules(WPD_STREAM_PKGCONF libwpd-stream-0.9)

find_path(WPD_INCLUDE_DIR
    NAMES libwpd/libwpd.h
    HINTS ${WPD_PKGCONF_INCLUDE_DIRS} ${WPD_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libwpd-0.9
)

find_path(WPD_STREAM_INCLUDE_DIR
    NAMES libwpd-stream/libwpd-stream.h
    HINTS ${WPD_STREAM_PKGCONF_INCLUDE_DIRS} ${WPD_STREAM_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libwpd-0.9
)

find_library(WPD_LIBRARY
    NAMES wpd libwpd wpd-0.9 libwpd-0.9
    HINTS ${WPD_PKGCONF_LIBRARY_DIRS} ${WPD_PKGCONF_LIBDIR}
)

find_library(WPD_STREAM_LIBRARY
    NAMES wpd-stream libwpd-stream wpd-stream-0.9 libwpd-stream-0.9
    HINTS ${WPD_STREAM_PKGCONF_LIBRARY_DIRS} ${WPD_STREAM_PKGCONF_LIBDIR}
)

set(LIBWPD_PROCESS_LIBS WPD_LIBRARY WPD_STREAM_LIBRARY)
set(LIBWPD_PROCESS_INCLUDES WPD_INCLUDE_DIR WPD_STREAM_INCLUDE_DIR)
libfind_process(LIBWPD)
