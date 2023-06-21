# SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
# SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindHEIF
--------------

Find HEIF headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``HEIF::heif``
  The HEIF library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``HEIF_FOUND``
  true if (the requested version of) HEIF is available.
``HEIF_VERSION``
  the version of HEIF.
``HEIF_LIBRARIES``
  the libraries to link against to use HEIF.
``HEIF_INCLUDE_DIRS``
  where to find the HEIF headers.
``HEIF_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

include(FindPackageHandleStandardArgs)

set(HEIF_PKGCONF_CONFIG_DIR CACHE STRING "PkgConfig path for locating the package modules")
foreach(_dir ${CMAKE_PREFIX_PATH})
    list(APPEND HEIF_PKGCONF_CONFIG_DIR ${_dir}/lib/cmake/libheif)
endforeach()
mark_as_advanced(HEIF_PKGCONF_CONFIG_DIR)

find_package(libheif QUIET NO_MODULE
    HINTS ${HEIF_PKGCONF_CONFIG_DIR} /usr/lib/cmake/libheif /usr/local/lib/cmake/libheif
)
mark_as_advanced(libheif_DIR)

# if we found the libheif CMake package then we are done, and
# can print what we found and return.
if(libheif_FOUND)
    if (TARGET heif AND NOT TARGET HEIF::heif)
        add_library(HEIF::heif ALIAS heif)
    endif()

    if (TARGET HEIF::HEIF)
        set(HEIF_FOUND ON)
    else ()
        set(HEIF_FOUND OFF)
    endif ()

    if (NOT HEIF_LIBRARIES)
        set(HEIF_LIBRARIES HEIF::heif)
    endif()
    unset(libheif_LIBRARIES)
    
    set(HEIF_VERSION "${libheif_VERSION}")
    unset(libheif_VERSION)

    get_target_property(HEIF_INCLUDE_DIRS HEIF::heif INTERFACE_INCLUDE_DIRECTORIES)

    find_package_handle_standard_args(HEIF 
        FOUND_VAR HEIF_FOUND
        REQUIRED_VARS HEIF_INCLUDE_DIRS HEIF_LIBRARIES
        VERSION_VAR HEIF_VERSION
        NAME_MISMATCHED
    )
    return()
endif()

find_package(PkgConfig QUIET)

if (PkgConfig_FOUND)
    pkg_check_modules(HEIF_PKGCONF QUIET libheif)
    set(HEIF_VERSION ${HEIF_PKGCONF_VERSION})
    set(HEIF_COMPILE_OPTIONS "${HEIF_PKGCONF_CFLAGS};${HEIF_PKGCONF_CFLAGS_OTHER}")
endif ()

find_path(HEIF_INCLUDE_DIR
    NAMES libheif/heif.h
    HINTS ${HEIF_PKGCONF_INCLUDE_DIRS} ${HEIF_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES heif
)

find_library(HEIF_LIBRARY
    NAMES libheif heif
    HINTS ${HEIF_PKGCONF_LIBRARY_DIRS} ${HEIF_PKGCONF_LIBDIR}
)

if (NOT HEIF_VERSION AND HEIF_FOUND)
    file(READ ${HEIF_INCLUDE_DIR}/libheif/heif_version.h _version_content)

    string(REGEX MATCH "#define LIBHEIF_VERSION[ \t]+\"(.+)\"" _version_match ${_version_content})

    if (_version_match)
        set(HEIF_VERSION "${CMAKE_MATCH_1}")
    else()
        if(NOT HEIF_FIND_QUIETLY)
            message(WARNING "Failed to get version information from ${HEIF_INCLUDE_DIR}/libheif/config.h")
        endif()
    endif()
endif()

if (HEIF_INCLUDE_DIR AND HEIF_LIBRARY)
    set(HEIF_FOUND ON)
else()
    set(HEIF_FOUND OFF)
endif()

find_package_handle_standard_args(HEIF
    FOUND_VAR HEIF_FOUND
    REQUIRED_VARS HEIF_INCLUDE_DIR HEIF_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR HEIF_VERSION
)

if (HEIF_FOUND)
if (HEIF_LIBRARY AND NOT TARGET HEIF::heif)
    add_library(HEIF::heif UNKNOWN IMPORTED GLOBAL)
    set_target_properties(HEIF::heif PROPERTIES
        IMPORTED_LOCATION "${HEIF_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${HEIF_PKGCONF_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${HEIF_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${HEIF_PKGCONF_LIBRARIES}"
        INTERFACE_LINK_DIRECTORIES "${HEIF_PKGCONF_LIBDIR}"
    )
endif()

mark_as_advanced(
    HEIF_INCLUDE_DIR
    HEIF_LIBRARY
)

set(HEIF_LIBRARIES ${HEIF_LIBRARY})
set(HEIF_INCLUDE_DIRS ${HEIF_INCLUDE_DIR})
endif()
