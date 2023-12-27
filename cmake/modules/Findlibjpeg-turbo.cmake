# SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindLibJPEG-Turbo
--------------

Find the libjpeg-turbo headers and libraries.

This Find Module complements the Config Module installed by the library, 
enabling detection of installed components.

For detecting libjpeg without the usage of CMake, please use FindJPEG.

Imported Targets
^^^^^^^^^^^^^^^^

``libjpeg-turbo::jpeg``
  The libjpeg drop-in replacement (shared) library, if found.

``libjpeg-turbo::jpeg-static``
  The libjpeg drop-in replacement (static) library, if found.

``libjpeg-turbo::turbojpeg``
  The TurboJPEG (shared) library, if found.

``libjpeg-turbo::turbojpeg-static``
  The TurboJPEG (static) library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``libjpeg-turbo_FOUND``
  true if (the requested version of) libjpeg-turbo is available.
``libjpeg-turbo_VERSION``
  the version of libjpeg-turbo.
``libjpeg-turbo_LIBRARIES``
  the libraries to link against to use libjpeg-turbo.
``libjpeg-turbo_INCLUDE_DIRS``
  where to find the libjpeg-turbo headers.
``libjpeg-turbo_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

include(FindPackageHandleStandardArgs)

find_package(libjpeg-turbo QUIET NO_MODULE)
mark_as_advanced(libjpeg-turbo_DIR)

# if we found the WebP CMake package then we are done, and
# can print what we found and return.
if(libjpeg-turbo_FOUND)
    if(TARGET libjpeg-turbo::jpeg)
        set(libjpeg-turbo_jpeg_FOUND ON)
    else()
        set(libjpeg-turbo_jpeg_FOUND OFF)
    endif()

    if(TARGET libjpeg-turbo::jpeg_static)
        set(libjpeg-turbo_jpeg_static_FOUND ON)
    else()
        set(libjpeg-turbo_jpeg_static_FOUND OFF)
    endif()

    if(TARGET libjpeg-turbo::turbojpeg)
        set(libjpeg-turbo_turbojpeg_FOUND ON)
    else()
        set(libjpeg-turbo_turbojpeg_FOUND OFF)
    endif()

    if(TARGET libjpeg-turbo::turbojpeg_static)
        set(libjpeg-turbo_turbojpeg_static_FOUND ON)
    else()
        set(libjpeg-turbo_turbojpeg_static_FOUND OFF)
    endif()

    find_package_handle_standard_args(libjpeg-turbo
        FOUND_VAR libjpeg-turbo_FOUND
        HANDLE_COMPONENTS
        CONFIG_MODE
    )
    return()
endif()

find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(PC_TURBOJPEG QUIET libturbojpeg)
    set(libjpeg-turbo_VERSION ${PC_TURBOJPEG_VERSION})
    set(libjpeg-turbo_COMPILE_OPTIONS "${PC_TURBOJPEG_CFLAGS} ${PC_TURBOJPEG_CFLAGS_OTHER}")

    pkg_check_modules(PC_JPEG QUIET libjpeg)
endif()

find_path(libjpeg-turbo_INCLUDE_DIR
    NAMES turbojpeg.h
    HINTS ${PC_TURBOJPEG_INCLUDEDIR} ${PC_TURBOJPEG_INCLUDE_DIRS}
)

find_library(libjpeg-turbo_LIBRARY
    NAMES ${PC_TURBOJPEG_LIBRARIES} turbojpeg
    HINTS ${PC_TURBOJPEG_LIBDIR} ${PC_TURBOJPEG_LIBRARY_DIRS}
)

# There's nothing in the TurboJPEG headers that could be used to detect the exact
# TurboJPEG version being used so don't attempt to do so. A version can only be found
# through pkg-config
if(NOT libjpeg-turbo_VERSION)
    message(WARNING "Cannot determine libjpeg-turbo version without pkg-config")
endif()

if(libjpeg-turbo_INCLUDE_DIR AND libjpeg-turbo_LIBRARY)
    set(libjpeg-turbo_turbojpeg_FOUND ON)
else()
    set(libjpeg-turbo_turbojpeg_FOUND OFF)
endif()

# Find components
if("turbojpeg_static" IN_LIST libjpeg-turbo_FIND_COMPONENTS)
    find_library(libjpeg-turbo_static_LIBRARY
        NAMES turbojpeg-static
        HINTS ${PC_TURBOJPEG_LIBDIR} ${PC_TURBOJPEG_LIBRARY_DIRS}
    )

    if(libjpeg-turbo_static_LIBRARY)
        set(libjpeg-turbo_turbojpeg_static_FOUND ON)
    else()
        set(libjpeg-turbo_turbojpeg_static_FOUND ON)
    endif()
endif()

if("jpeg" IN_LIST libjpeg-turbo_FIND_COMPONENTS)
    find_library(jpeg_LIBRARY
        NAMES jpeg
        HINTS ${PC_JPEG_LIBDIR} ${PC_JPEG_LIBRARY_DIRS}
    )

    if(jpeg_LIBRARY)
        set(libjpeg-turbo_jpeg_FOUND ON)
    else()
        set(libjpeg-turbo_jpeg_FOUND OFF)
    endif()
endif()

if("jpeg_static" IN_LIST libjpeg-turbo_FIND_COMPONENTS)
    find_library(jpeg_static_LIBRARY
        NAMES jpeg-static
        HINTS ${PC_JPEG_LIBDIR} ${PC_JPEG_LIBRARY_DIRS}
    )

    if(jpeg_static_LIBRARY)
        set(libjpeg-turbo_jpeg_static_FOUND ON)
    else()
        set(libjpeg-turbo_jpeg_static_FOUND OFF)
    endif()
endif()

find_package_handle_standard_args(libjpeg-turbo
    FOUND_VAR libjpeg-turbo_FOUND
    REQUIRED_VARS libjpeg-turbo_INCLUDE_DIR libjpeg-turbo_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR libjpeg-turbo_VERSION
)

if(libjpeg-turbo_FOUND)
    if(libjpeg-turbo_LIBRARY AND NOT TARGET libjpeg-turbo::turbojpeg)
        add_library(libjpeg-turbo::turbojpeg UNKNOWN IMPORTED GLOBAL)
        set_target_properties(libjpeg-turbo::turbojpeg PROPERTIES
            IMPORTED_LOCATION "${libjpeg-turbo_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_TURBOJPEG_CFLAGS} ${PC_TURBOJPEG_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${libjpeg-turbo_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${PC_TURBOJPEG_LINK_LIBRARIES}"
        )
    endif()

    if(libjpeg-turbo_static_LIBRARY AND NOT TARGET libjpeg-turbo::turbojpeg_static)
        add_library(libjpeg-turbo::turbojpeg_static UNKNOWN IMPORTED GLOBAL)
        set_target_properties(libjpeg-turbo::turbojpeg_static PROPERTIES
            IMPORTED_LOCATION "${libjpeg-turbo_static_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_TURBOJPEG_CFLAGS} ${PC_TURBOJPEG_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${libjpeg-turbo_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${PC_TURBOJPEG_LINK_LIBRARIES}"
        )
    endif()

    if(jpeg_LIBRARY AND NOT TARGET libjpeg-turbo::jpeg)
        add_library(libjpeg-turbo::jpeg UNKNOWN IMPORTED GLOBAL)
        set_target_properties(libjpeg-turbo::jpeg PROPERTIES
            IMPORTED_LOCATION "${jpeg_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_JPEG_CFLAGS} ${PC_JPEG_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${libjpeg-turbo_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${PC_JPEG_LINK_LIBRARIES}"
        )
    endif()

    if(jpeg_static_LIBRARY AND NOT TARGET libjpeg-turbo::jpeg_static)
        add_library(libjpeg-turbo::jpeg_static UNKNOWN IMPORTED GLOBAL)
        set_target_properties(libjpeg-turbo::jpeg_static PROPERTIES
            IMPORTED_LOCATION "${jpeg_static_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_JPEG_CFLAGS} ${PC_JPEG_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${libjpeg-turbo_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${PC_JPEG_LINK_LIBRARIES}"
        )
    endif()

    mark_as_advanced(
        libjpeg-turbo_INCLUDE_DIR
        libjpeg-turbo_LIBRARY
        libjpeg-turbo_static_LIBRARY
        jpeg_LIBRARY
        jpeg_static_LIBRARY
    )

    set(libjpeg-turbo_LIBRARIES ${libjpeg-turbo_LIBRARY} ${jpeg_LIBRARY})
    set(libjpeg-turbo_INCLUDE_DIRS ${libjpeg-turbo_INCLUDE_DIR})
endif()
