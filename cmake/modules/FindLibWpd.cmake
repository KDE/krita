# - Try to find the libwpd (WordPerfect library)
# Once done this will define
#
#  LIBWPD_FOUND - system has LIBWPD
#  LIBWPD_INCLUDE_DIRS - the LIBWPD include directory
#  LIBWPD_LIBRARIES - Link these to use LIBWPD
#  LIBWPD_DEFINITIONS - Compiler switches required for using LIBWPD
#

include(LibFindMacros)
libfind_pkg_check_modules(WPD_PKGCONF libwpd-0.10)

find_path(WPD_INCLUDE_DIR
    NAMES libwpd/libwpd.h
    HINTS ${WPD_PKGCONF_INCLUDE_DIRS} ${WPD_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libwpd-0.10
)

find_library(WPD_LIBRARY
    NAMES wpd libwpd wpd-0.10 libwpd-0.10
    HINTS ${WPD_PKGCONF_LIBRARY_DIRS} ${WPD_PKGCONF_LIBDIR}
)

set(LIBWPD_PROCESS_LIBS WPD_LIBRARY)
set(LIBWPD_PROCESS_INCLUDES WPD_INCLUDE_DIR)
libfind_process(LIBWPD)
