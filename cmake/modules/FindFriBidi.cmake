# SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
# SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindFriBidi
--------------

Find FriBidi headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``FriBidi::FriBidi``
  The FriBidi library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``FriBidi_FOUND``
  true if (the requested version of) FriBidi is available.
``FriBidi_VERSION``
  the version of FriBidi.
``FriBidi_LIBRARIES``
  the libraries to link against to use FriBidi.
``FriBidi_INCLUDE_DIRS``
  where to find the FriBidi headers.
``FriBidi_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)

if (PkgConfig_FOUND)
    pkg_check_modules(PC_FRIBIDI QUIET fribidi)
    set(FriBidi_VERSION ${PC_FRIBIDI_VERSION})
    set(FriBidi_COMPILE_OPTIONS "${PC_FRIBIDI_CFLAGS} ${PC_FRIBIDI_CFLAGS_OTHER}")
endif ()

find_path(FriBidi_INCLUDE_DIR
    NAMES fribidi/fribidi.h
    HINTS ${PC_FRIBIDI_INCLUDEDIR} ${PC_FRIBIDI_INCLUDE_DIRS}
)
if(FriBidi_INCLUDE_DIR)
    set(FriBidi_INCLUDE_DIR ${FriBidi_INCLUDE_DIR}/fribidi)
endif()

find_library(FriBidi_LIBRARY
    NAMES ${FriBidi_NAMES} fribidi
    HINTS ${PC_FRIBIDI_LIBDIR} ${PC_FRIBIDI_LIBRARY_DIRS}
)

if (NOT FriBidi_VERSION AND FriBidi_INCLUDE_DIR)
    file(READ ${FriBidi_INCLUDE_DIR}/fribidi-config.h _fribidi_version_content)

    string(REGEX MATCH "#define FRIBIDI_VERSION[ \t]+\"([0-9.]+)\"\n" _version_match ${_fribidi_version_content})

    if (_version_match)
        set(FriBidi_VERSION "${CMAKE_MATCH_1}")
    else()
        if(NOT FriBidi_FIND_QUIETLY)
            message(WARNING "Failed to get version information from ${FriBidi_INCLUDE_DIR}/fribidi-config.h")
        endif()
    endif()
endif()

if (FriBidi_INCLUDE_DIR AND FriBidi_LIBRARY)
    set(FriBidi_FOUND ON)
else()
    set(FriBidi_FOUND OFF)
endif()

find_package_handle_standard_args(FriBidi
    FOUND_VAR FriBidi_FOUND
    REQUIRED_VARS FriBidi_INCLUDE_DIR FriBidi_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR FriBidi_VERSION
)

if (FriBidi_FOUND)
if (FriBidi_LIBRARY AND NOT TARGET FriBidi::FriBidi)
    add_library(FriBidi::FriBidi UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FriBidi::FriBidi PROPERTIES
        IMPORTED_LOCATION "${FriBidi_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_FRIBIDI_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FriBidi_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${PC_FRIBIDI_LINK_LIBRARIES}"
        INTERFACE_LINK_DIRECTORIES "${PC_FRIBIDI_LIBDIR}"
    )
endif ()

mark_as_advanced(
    FriBidi_INCLUDE_DIR
    FriBidi_LIBRARY
)

set(FriBidi_LIBRARIES ${FriBidi_LIBRARY})
set(FriBidi_INCLUDE_DIRS ${FriBidi_INCLUDE_DIR})
endif()
