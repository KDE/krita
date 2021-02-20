#For finding LibMyPaint library in the system
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(LibMyPaint ${LibMyPaint_FIND_VERSION} QUIET NO_MODULE)

if(LibMyPaint_FOUND)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibMyPaint
    CONFIG_MODE
    REQUIRED_VARS
        libmypaint_INCLUDE_DIRS
        libmypaint_LIBRARY_DIRS
    VERSION_VAR
        libmypaint_VERSION
)

else()

include(LibFindMacros)
libfind_pkg_check_modules(MYPAINT_PKGCONF libmypaint)

find_path(LibMyPaint_INCLUDE_DIRS
    NAMES libmypaint/mypaint-config.h
    HINTS ${MYPAINT_PKGCONF_INCLUDE_DIRS} ${MYPAINT_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libmypaint
)

find_library(LibMyPaint_LIBRARY
    NAMES libmypaint mypaint
    HINTS ${MYPAINT_PKGCONF_LIBRARY_DIRS} ${MYPAINT_PKGCONF_LIBDIR}
    DOC "Libraries to link against for mypaint brush engine Support"
)

set(LibMyPaint_PROCESS_LIBS LibMyPaint_LIBRARY)
set(LibMyPaint_PROCESS_INCLUDES LibMyPaint_INCLUDE_DIRS)
set(LibMyPaint_VERSION ${MYPAINT_PKGCONF_VERSION})
libfind_process(LibMyPaint)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibMyPaint
    REQUIRED_VARS
        LibMyPaint_INCLUDE_DIRS
        LibMyPaint_LIBRARY
    VERSION_VAR
        LibMyPaint_VERSION
)
endif()
