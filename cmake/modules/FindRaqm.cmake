# SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindRaqm
--------------

Find Raqm headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``Raqm::Raqm``
  The Raqm library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``Raqm_FOUND``
  true if (the requested version of) Raqm is available.
``Raqm_VERSION``
  the version of Raqm.
``Raqm_LIBRARIES``
  the libraries to link against to use Raqm.
``Raqm_INCLUDE_DIRS``
  where to find the Raqm headers.
``Raqm_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)

if (PkgConfig_FOUND)
    pkg_check_modules(PC_RAQM QUIET raqm)
    set(Raqm_VERSION ${PC_RAQM_VERSION})
    set(Raqm_COMPILE_OPTIONS "${PC_RAQM_CFLAGS} ${PC_RAQM_CFLAGS_OTHER}")
endif ()

find_path(Raqm_INCLUDE_DIR
    NAMES raqm.h
    HINTS ${PC_RAQM_INCLUDEDIR} ${PC_RAQM_INCLUDE_DIRS}
)

find_library(Raqm_LIBRARY
    NAMES ${Raqm_NAMES} raqm
    HINTS ${PC_RAQM_LIBDIR} ${PC_RAQM_LIBRARY_DIRS}
)

if (NOT Raqm_VERSION AND Raqm_INCLUDE_DIR)
    file(READ ${Raqm_INCLUDE_DIR}/raqm-version.h _raqm_version_content)

    string(REGEX MATCH "#define RAQM_VERSION_STRING[ \t]+\"([0-9.]+)\"\n" _version_match ${_raqm_version_content})

    if (_version_match)
        set(Raqm_VERSION "${CMAKE_MATCH_1}")
    else()
        if(NOT Raqm_FIND_QUIETLY)
            message(WARNING "Failed to get version information from ${Raqm_INCLUDE_DIR}/raqm-version.h")
        endif()
    endif()
endif()

if (Raqm_INCLUDE_DIR AND Raqm_LIBRARY)
    set(Raqm_FOUND ON)
else()
    set(Raqm_FOUND OFF)
endif()

find_package_handle_standard_args(Raqm
    FOUND_VAR Raqm_FOUND
    REQUIRED_VARS Raqm_INCLUDE_DIR Raqm_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR Raqm_VERSION
)

if (Raqm_FOUND)
if (Raqm_LIBRARY AND NOT TARGET Raqm::Raqm)
    add_library(Raqm::Raqm UNKNOWN IMPORTED GLOBAL)
    set_target_properties(Raqm::Raqm PROPERTIES
        IMPORTED_LOCATION "${Raqm_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_RAQM_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Raqm_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${PC_RAQM_LINK_LIBRARIES}"
        INTERFACE_LINK_DIRECTORIES "${PC_RAQM_LIBDIR}"
    )
endif ()

mark_as_advanced(
    Raqm_INCLUDE_DIR
    Raqm_LIBRARY
)

set(Raqm_LIBRARIES ${Raqm_LIBRARY})
set(Raqm_INCLUDE_DIRS ${Raqm_INCLUDE_DIR})
endif()
