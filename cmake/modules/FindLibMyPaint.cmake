#For finding LibMyPaint library in the system

include(LibFindMacros)
libfind_pkg_check_modules(MYPAINT_PKGCONF libmypaint)

find_path(LIBMYPAINT_INCLUDE_DIR
    NAMES libmypaint/mypaint-brush.h
    HINTS ${MYPAINT_PKGCONF_INCLUDE_DIRS} ${MYPAINT_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libmypaint
)

find_library(LIBMYPAINT_LIBRARY
    NAMES libmypaint mypaint
    HINTS ${MYPAINT_PKGCONF_LIBRARY_DIRS} ${MYPAINT_PKGCONF_LIBDIR}
    DOC "Libraries to link against for mypaint brush engine Support"
)

set(LIBMYPAINT_FOUND ${MYPAINT_PKGCONF_FOUND})
set(LIBMYPAINT_VERSION ${MYPAINT_PKGCONF_VERSION})
