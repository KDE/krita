# SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
Findlibunibreak
--------------

Find libunibreak headers and library.

Imported Targets
^^^^^^^^^^^^^^^^

``libunibreak::libunibreak``
  The libunibreak library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``libunibreak_FOUND``
  true if (the requested version of) libunibreak is available.
``libunibreak_VERSION``
  the version of libunibreak.
``libunibreak_LIBRARIES``
  the libraries to link against to use libunibreak.
``libunibreak_INCLUDE_DIRS``
  where to find the libunibreak headers.
``libunibreak_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)

if (PkgConfig_FOUND)
    pkg_check_modules(PC_LIBUNIBREAK QUIET libunibreak)
    set(libunibreak_VERSION ${PC_LIBUNIBREAK_VERSION})
    set(libunibreak_COMPILE_OPTIONS "${PC_LIBUNIBREAK_CFLAGS} ${PC_LIBUNIBREAK_CFLAGS_OTHER}")
endif ()

find_path(libunibreak_INCLUDE_DIR
    NAMES unibreakbase.h
    HINTS ${PC_LIBUNIBREAK_INCLUDEDIR} ${PC_LIBUNIBREAK_INCLUDE_DIRS}
)

find_library(libunibreak_LIBRARY
    NAMES ${libunibreak_NAMES} unibreak libunibreak
    HINTS ${PC_LIBUNIBREAK_LIBDIR} ${PC_LIBUNIBREAK_LIBRARY_DIRS}
)

if (NOT libunibreak_VERSION)
    file(READ ${libunibreak_INCLUDE_DIR}/unibreakbase.h _libunibreak_version_content)

    string(REGEX MATCH "#define UNIBREAK_VERSION[ \t]+0x([0-9][0-9])([0-9][0-9])[^\n]*\n" _version_match ${_libunibreak_version_content})

    if (_version_match)
        set(libunibreak_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
    else()
        if(NOT libunibreak_FIND_QUIETLY)
            message(WARNING "Failed to get version information from ${libunibreak_INCLUDE_DIR}/unibreakbase.h")
        endif()
    endif()
endif()

if (libunibreak_INCLUDE_DIR AND libunibreak_LIBRARY)
    set(libunibreak_FOUND ON)
else()
    set(libunibreak_FOUND OFF)
endif()

find_package_handle_standard_args(libunibreak
    FOUND_VAR libunibreak_FOUND
    REQUIRED_VARS libunibreak_INCLUDE_DIR libunibreak_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR libunibreak_VERSION
)

if (libunibreak_FOUND)
if (libunibreak_LIBRARY AND NOT TARGET libunibreak::libunibreak)
    add_library(libunibreak::libunibreak UNKNOWN IMPORTED GLOBAL)
    set_target_properties(libunibreak::libunibreak PROPERTIES
        IMPORTED_LOCATION "${libunibreak_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_LIBUNIBREAK_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${libunibreak_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${PC_LIBUNIBREAK_LINK_LIBRARIES}"
        INTERFACE_LINK_DIRECTORIES "${PC_LIBUNIBREAK_LIBDIR}"
    )
endif ()

mark_as_advanced(
    libunibreak_INCLUDE_DIR
    libunibreak_LIBRARY
)

set(libunibreak_LIBRARIES ${libunibreak_LIBRARY})
set(libunibreak_INCLUDE_DIRS ${libunibreak_INCLUDE_DIR})
endif()
