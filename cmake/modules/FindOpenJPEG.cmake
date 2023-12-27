# SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause
#
#[=======================================================================[.rst:
FindOpenJPEG
--------------

Find OpenJPEG headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``OpenJPEG::openjp2``
  The OpenJPEG library, if found.

``OpenJPEG::opj_decompress``
  The OpenJPEG decompression executable, if found.

``OpenJPEG::opj_compress``
  The OpenJPEG compression executable, if found.

``OpenJPEG::opj_dump``
  The OpenJPEG dump executable, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``OpenJPEG_FOUND``
  true if (the requested version of) OpenJPEG is available.
``OpenJPEG_VERSION``
  the version of OpenJPEG.
``OpenJPEG_LIBRARIES``
  the libraries to link against to use OpenJPEG.
``OpenJPEG_INCLUDE_DIRS``
  where to find the OpenJPEG headers.
``OpenJPEG_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

include(FindPackageHandleStandardArgs)

# No hints in this case because the library versions semantically
# the module folder, and it's also capitalized, so let CMake handle it.
find_package(OPENJPEG QUIET NO_MODULE NAMES OpenJPEG)
mark_as_advanced(OPENJPEG_DIR)

if(OPENJPEG_FOUND)
    if (TARGET openjp2 AND NOT TARGET OpenJPEG::openjp2)
        add_library(OpenJPEG::openjp2 ALIAS openjp2)
    endif()

    if (TARGET OpenJPEG::openjp2)
        set(OpenJPEG_openjp2_FOUND ON)
    else ()
        set(OpenJPEG_openjp2_FOUND OFF)
    endif ()

    if (TARGET opj_decompress AND NOT TARGET OpenJPEG::opj_decompress)
        add_executable(OpenJPEG::opj_decompress ALIAS opj_decompress)
    endif()

    if (TARGET OpenJPEG::opj_decompress)
        set(OpenJPEG_opj_decompress_FOUND ON)
    else ()
        set(OpenJPEG_opj_decompress_FOUND OFF)
    endif ()

    if (TARGET opj_compress AND NOT TARGET OpenJPEG::opj_compress)
        add_executable(OpenJPEG::opj_compress ALIAS opj_compress)
    endif()

    if (TARGET OpenJPEG::opj_compress)
        set(OpenJPEG_opj_compress_FOUND ON)
    else ()
        set(OpenJPEG_opj_compress_FOUND OFF)
    endif ()

    if (TARGET opj_dump AND NOT TARGET OpenJPEG::opj_dump)
        add_executable(OpenJPEG::opj_dump ALIAS opj_dump)
    endif()

    if (TARGET OpenJPEG::opj_dump)
        set(OpenJPEG_opj_dump_FOUND ON)
    else ()
        set(OpenJPEG_opj_dump_FOUND OFF)
    endif ()

    if (TARGET openjpip AND NOT TARGET OpenJPEG::openjpip)
        add_library(OpenJPEG::openjpip ALIAS openjpip)
    endif()

    if (TARGET OpenJPEG::openjpip)
        set(OpenJPEG_openjpip_FOUND ON)
    else ()
        set(OpenJPEG_openjpip_FOUND OFF)
    endif ()

    if (TARGET openjpip_server AND NOT TARGET OpenJPEG::openjpip_server)
        add_library(OpenJPEG::openjpip_server ALIAS openjpip_server)
    endif()

    if (TARGET OpenJPEG::openjpip_server)
        set(OpenJPEG_openjpip_server_FOUND ON)
    else ()
        set(OpenJPEG_openjpip_server_FOUND OFF)
    endif ()

    # Patch in the version string, if needed.
    if (NOT OpenJPEG_VERSION)
        set(OpenJPEG_VERSION "${OPENJPEG_MAJOR_VERSION}.${OPENJPEG_MINOR_VERSION}.${OPENJPEG_BUILD_VERSION}")
    endif ()

    # Patch in the library and include dirs.
    if (NOT OpenJPEG_LIBRARIES)
        set(OpenJPEG_LIBRARIES OpenJPEG::openjp2)
    endif()
    set(OpenJPEG_INCLUDE_DIRS "${OPENJPEG_INCLUDE_DIRS}")

    find_package_handle_standard_args(OpenJPEG 
        FOUND_VAR OpenJPEG_FOUND
        REQUIRED_VARS OpenJPEG_INCLUDE_DIRS OpenJPEG_LIBRARIES
        HANDLE_COMPONENTS
        VERSION_VAR OpenJPEG_VERSION
        NAME_MISMATCHED
        # Don't, it defines mismatching cased variables.
        # CONFIG_MODE
    )
    return()
endif()

find_package(PkgConfig QUIET)

if (PkgConfig_FOUND)
    pkg_check_modules(PC_OPENJPEG QUIET libopenjp2)
    set(OpenJPEG_VERSION ${PC_OPENJPEG_VERSION})
    set(OpenJPEG_COMPILE_OPTIONS "${PC_OPENJPEG_CFLAGS};${PC_OPENJPEG_CFLAGS_OTHER}")

    pkg_check_modules(PC_OPENJPIP QUIET openjpip)
endif ()

find_path(OpenJPEG_INCLUDE_DIR
    NAMES openjpeg.h
    HINTS ${PC_OPENJPEG_INCLUDEDIR} ${PC_OPENJPEG_INCLUDE_DIRS}
)

find_library(OpenJPEG_LIBRARY
    NAMES openjp2 libopenjp2
    HINTS ${PC_OPENJPEG_LIBDIR} ${PC_OPENJPEG_LIBRARY_DIRS}
)

# There's nothing in the OpenJPEG headers that could be used to detect the exact
# OpenJPEG version being used so don't attempt to do so. A version can only be found
# through pkg-config
if (NOT OpenJPEG_VERSION)
    message(WARNING "Cannot determine OpenJPEG version without pkg-config")
endif ()

if (OpenJPEG_INCLUDE_DIR AND OpenJPEG_LIBRARY)
    set(OpenJPEG_openjp2_FOUND ON)
else()
    set(OpenJPEG_openjp2_FOUND OFF)
endif()

# Find components
if ("openjpip" IN_LIST OpenJPEG_FIND_COMPONENTS)
    find_library(OpenJPEG_openjpip_LIBRARY
        NAMES openjpip libopenjpip
        HINTS ${PC_OPENJPIP_LIBDIR} ${PC_OPENJPIP_LIBRARY_DIRS}
    )

    if (OpenJPEG_openjpip_LIBRARY)
        set(OpenJPEG_openjpip_FOUND ON)
    else ()
        set(OpenJPEG_openjpip_FOUND OFF)
    endif ()
endif ()

if ("openjpip_server" IN_LIST OpenJPEG_FIND_COMPONENTS)
    find_library(OpenJPEG_openjpip_server_LIBRARY
        NAMES openjpip_server libopenjpip_server
        HINTS ${PC_OPENJPIP_LIBDIR} ${PC_OPENJPIP_LIBRARY_DIRS}
    )

    if (OpenJPEG_openjpip_server_LIBRARY)
        set(OpenJPEG_openjpip_server_FOUND ON)
    else ()
        set(OpenJPEG_openjpip_server_FOUND OFF)
    endif ()
endif ()

find_package_handle_standard_args(OpenJPEG
    FOUND_VAR OpenJPEG_FOUND
    REQUIRED_VARS OpenJPEG_INCLUDE_DIR OpenJPEG_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR OpenJPEG_VERSION
)

if (OpenJPEG_FOUND)
if (OpenJPEG_LIBRARY AND NOT TARGET OpenJPEG::openjp2)
    add_library(OpenJPEG::openjp2 UNKNOWN IMPORTED GLOBAL)
    set_target_properties(OpenJPEG::openjp2 PROPERTIES
        IMPORTED_LOCATION "${OpenJPEG_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_OPENJPEG_CFLAGS};${PC_OPENJPEG_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenJPEG_INCLUDE_DIR};${PC_OPENJPEG_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PC_OPENJPEG_LINK_LIBRARIES}"
    )
endif ()

if (OpenJPEG_openjpip_LIBRARY AND NOT TARGET OpenJPEG::openjpip)
    add_library(OpenJPEG::openjpip UNKNOWN IMPORTED GLOBAL)
    set_target_properties(OpenJPEG::openjpip PROPERTIES
        IMPORTED_LOCATION "${OpenJPEG_openjpip_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_OPENJPIP_CFLAGS};${PC_OPENJPIP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenJPEG_INCLUDE_DIR};${PC_OPENJPIP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PC_OPENJPIP_LINK_LIBRARIES}"
    )
endif ()

if (OpenJPEG_openjpip_server_LIBRARY AND NOT TARGET OpenJPEG::openjpip_server)
    add_library(OpenJPEG::openjpip_server UNKNOWN IMPORTED GLOBAL)
    set_target_properties(OpenJPEG::openjpip_server PROPERTIES
        IMPORTED_LOCATION "${OpenJPEG_openjpip_server_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_OPENJPIP_CFLAGS};${PC_OPENJPIP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenJPEG_INCLUDE_DIR};${PC_OPENJPIP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PC_OPENJPIP_LINK_LIBRARIES}"
    )
endif ()

mark_as_advanced(
    OpenJPEG_INCLUDE_DIR
    OpenJPEG_LIBRARY
    OpenJPEG_openjpip_LIBRARY
    OpenJPEG_openjpip_server_LIBRARY
)

set(OpenJPEG_LIBRARIES ${OpenJPEG_LIBRARY} ${OpenJPEG_openjpip_LIBRARY} ${OpenJPEG_openjpip_server_LIBRARY})
set(OpenJPEG_INCLUDE_DIRS ${OpenJPEG_INCLUDE_DIR})

endif()
