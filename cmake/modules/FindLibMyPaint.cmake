#For finding LibMyPaint library in the system

include(LibFindMacros)
libfind_pkg_check_modules(MYPAINT_PKGCONF libmypaint)

find_path(LIBMYPAINT_INCLUDE_DIR
    NAMES mypaint-config.h
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${MYPAINT_PKGCONF_INCLUDE_DIRS}
    ${MYPAINT_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libmypaint
)

find_library(LIBMYPAINT_LIBRARY
    NAMES libmypaint mypaint
    HINTS ${MYPAINT_PKGCONF_LIBRARY_DIRS} ${MYPAINT_PKGCONF_LIBDIR}
    DOC "Libraries to link against for mypaint brush engine Support"
)

string(REGEX MATCH "(.*)/libmypaint.so" LIBMYPAINT_LIBRARIES ${LIBMYPAINT_LIBRARY})

set(LIBMYPAINT_LIBRARIES ${CMAKE_MATCH_1})
set(LIBMYPAINT_FOUND ${MYPAINT_PKGCONF_FOUND})
set(LIBMYPAINT_VERSION ${MYPAINT_PKGCONF_VERSION})
