# SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
# SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindLibMyPaint
--------------

Find LibMyPaint headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``LibMyPaint::mypaint``
  The LibMyPaint library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``LibMyPaint_FOUND``
  true if (the requested version of) LibMyPaint is available.
``LibMyPaint_VERSION``
  the version of LibMyPaint.
``LibMyPaint_LIBRARIES``
  the libraries to link against to use LibMyPaint.
``LibMyPaint_INCLUDE_DIRS``
  where to find the LibMyPaint headers.
``LibMyPaint_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]


set(MYPAINT_PKGCONF_CONFIG_DIR CACHE STRING "PkgConfig path for locating the package modules")
foreach(_dir ${CMAKE_PREFIX_PATH})
    list(APPEND MYPAINT_PKGCONF_CONFIG_DIR ${_dir}/lib/cmake/libmypaint)
endforeach()
mark_as_advanced(MYPAINT_PKGCONF_CONFIG_DIR)

find_package(libmypaint QUIET NO_MODULE
    HINTS ${MYPAINT_PKGCONF_CONFIG_DIR} /usr/lib/cmake/libmypaint /usr/local/lib/cmake/libmypaint
)
mark_as_advanced(libmypaint_DIR)

if(libmypaint_FOUND)

    # Patch in the missing definitions.
    if (TARGET mypaint AND NOT TARGET LibMyPaint::mypaint)
        add_library(LibMyPaint::mypaint ALIAS mypaint)
    endif()

    if (NOT Libmypaint_LIBRARIES)
        set(LibMyPaint_LIBRARIES LibMyPaint::mypaint)
    endif()

    set(LibMyPaint_INCLUDE_DIRS "${libmypaint_INCLUDE_DIRS}")
    set(LibMyPaint_VERSION "${libmypaint_VERSION}")

    find_package_handle_standard_args(LibMyPaint
        FOUND_VAR LibMyPaint_FOUND
        REQUIRED_VARS LibMyPaint_INCLUDE_DIRS LibMyPaint_LIBRARIES
        VERSION_VAR LibMyPaint_VERSION
        NAME_MISMATCHED
        # CONFIG_MODE
    )

    return()
endif()
    
find_package(PkgConfig QUIET)

if (PkgConfig_FOUND)
    pkg_check_modules(MYPAINT_PKGCONF QUIET libmypaint)
    set(LibMyPaint_VERSION ${MYPAINT_PKGCONF_VERSION})
    set(LibMyPaint_COMPILE_OPTIONS "${MYPAINT_PKGCONF_CFLAGS};${MYPAINT_PKGCONF_CFLAGS_OTHER}")
endif ()

find_path(LibMyPaint_INCLUDE_DIR
    NAMES libmypaint/mypaint-config.h
    HINTS ${MYPAINT_PKGCONF_INCLUDE_DIRS} ${MYPAINT_PKGCONF_INCLUDEDIR}
    PATH_SUFFIXES libmypaint
)

find_library(LibMyPaint_LIBRARY
    NAMES libmypaint mypaint
    HINTS ${MYPAINT_PKGCONF_LIBRARY_DIRS} ${MYPAINT_PKGCONF_LIBDIR}
)

if (NOT LibMyPaint_VERSION)
    file(READ ${LibMyPaint_INCLUDE_DIR}/config.h _version_content)

    string(REGEX MATCH "#define PACKAGE_VERSION[ \t]+\"(.+)\"" _version_match ${_version_content})

    if (_version_match)
        set(LibMyPaint_VERSION "${CMAKE_MATCH_1}")
    else()
        if(NOT LibMyPaint_FIND_QUIETLY)
            message(WARNING "Failed to get version information from ${LibMyPaint_INCLUDE_DIR}/config.h")
        endif()
    endif()
endif()

if (LibMyPaint_INCLUDE_DIR AND LibMyPaint_LIBRARY)
    set(LibMyPaint_FOUND ON)
else()
    set(LibMyPaint_FOUND OFF)
endif()

find_package_handle_standard_args(LibMyPaint
    FOUND_VAR LibMyPaint_FOUND
    REQUIRED_VARS LibMyPaint_INCLUDE_DIR LibMyPaint_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR LibMyPaint_VERSION
)

if (LibMyPaint_FOUND)
if (LibMyPaint_LIBRARY AND NOT TARGET LibMyPaint::mypaint)
    add_library(LibMyPaint::mypaint UNKNOWN IMPORTED GLOBAL)
    set_target_properties(LibMyPaint::mypaint PROPERTIES
        IMPORTED_LOCATION "${LibMyPaint_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${MYPAINT_PKGCONF_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${LibMyPaint_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${MYPAINT_PKGCONF_LIBRARIES}"
        INTERFACE_LINK_DIRECTORIES "${MYPAINT_PKGCONF_LIBDIR}"
    )
endif()

mark_as_advanced(
    LibMyPaint_INCLUDE_DIR
    LibMyPaint_LIBRARY
)

set(LibMyPaint_LIBRARIES ${LibMyPaint_LIBRARY})
set(LibMyPaint_INCLUDE_DIRS ${LibMyPaint_INCLUDE_DIR})
endif()
