# - Try to find the libheif library
# Once done this will define
#
#  HEIF_FOUND - system has heif
#  HEIF_INCLUDE_DIRS - the heif include directories
#  HEIF_LIBRARIES - the libraries needed to use heif
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
if (NOT WIN32)
    include(LibFindMacros)
    libfind_pkg_check_modules(HEIF_PKGCONF libheif)

    find_path(HEIF_INCLUDE_DIR
        NAMES libheif/heif.h
        HINTS ${HEIF_PKGCONF_INCLUDE_DIRS} ${HEIF_PKGCONF_INCLUDEDIR}
        PATH_SUFFIXES heif
    )

    find_library(HEIF_LIBRARY
        NAMES heif
        HINTS ${HEIF_PKGCONF_LIBRARY_DIRS} ${HEIF_PKGCONF_LIBDIR}
    )

    set(HEIF_PROCESS_LIBS HEIF_LIBRARY)
    set(HEIF_PROCESS_INCLUDES HEIF_INCLUDE_DIR)
    libfind_process(HEIF)

else()
    find_path(HEIF_INCLUDE_DIR
        NAMES heif.h
    )

    find_library (
        HEIF_LIBRARY
        NAMES libheif libheif
        DOC "Libraries to link against for HEIF Support"
    )

    if (HEIF_LIBRARY)
        set(HEIF_LIBRARY_DIR ${HEIF_LIBRARY})
    endif()

    set (HEIF_LIBRARIES ${HEIF_LIBRARY})

    if(HEIF_INCLUDE_DIR AND HEIF_LIBRARY_DIR)
        set (HEIF_FOUND true)
    endif()
endif()

