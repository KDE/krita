# Copyright (C) 2020 Sony Interactive Entertainment Inc.
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
# Copyright (C) 2013 Igalia S.L.
# Copyright (C) 2021 L. E. Segovia <amy@amyspark.me>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#[=======================================================================[.rst:
FindWebP
--------------

Find WebP headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``WebP::webp``
  The WebP library, if found.

``WebP::webpdemux``
  The WebP demux library, if found.

``WebP::libwebpmux``
  The WebP mux library, if found.

``WebP::decoder``
  The WebP decoder library, if found.

``WebP::sharpyuv``
  The WebP Sharp RGB to YUV420 conversion library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``WebP_FOUND``
  true if (the requested version of) WebP is available.
``WebP_VERSION``
  the version of WebP.
``WebP_LIBRARIES``
  the libraries to link against to use WebP.
``WebP_INCLUDE_DIRS``
  where to find the WebP headers.
``WebP_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

include(FindPackageHandleStandardArgs)

set(PC_WEBP_CONFIG_DIR CACHE STRING "PkgConfig path for locating the package modules")
foreach(_dir ${CMAKE_PREFIX_PATH})
    list(APPEND PC_WEBP_CONFIG_DIR ${_dir}/share/WebP/cmake)
endforeach()
mark_as_advanced(PC_WEBP_CONFIG_DIR)

find_package(WebP QUIET NO_MODULE
    HINTS ${PC_WEBP_CONFIG_DIR} /usr/share/WebP/cmake /usr/local/share/WebP/cmake
)
mark_as_advanced(WebP_DIR)

# if we found the WebP CMake package then we are done, and
# can print what we found and return.
if(WebP_FOUND)
    if (TARGET WebP::webp)
        set(WebP_webp_FOUND ON)
    else ()
        set(WebP_webp_FOUND OFF)
    endif ()

    if (TARGET WebP::webpdemux)
        set(WebP_demux_FOUND ON)
    else ()
        set(WebP_demux_FOUND OFF)
    endif ()

    if (TARGET WebP::libwebpmux)
        set(WebP_mux_FOUND ON)
    else ()
        set(WebP_mux_FOUND OFF)
    endif ()

    if (TARGET WebP::decoder)
        set(WebP_decoder_FOUND ON)
    else ()
        set(WebP_decoder_FOUND OFF)
    endif ()

    if (TARGET WebP::sharpyuv)
        set(WebP_sharpyuv_FOUND ON)
    else ()
        set(WebP_sharpyuv_FOUND OFF)
    endif ()

    find_package_handle_standard_args(WebP 
        FOUND_VAR WebP_FOUND
        HANDLE_COMPONENTS
        CONFIG_MODE
    )
    return()
endif()

find_package(PkgConfig QUIET)

if (PkgConfig_FOUND)
    pkg_check_modules(PC_WEBP QUIET libwebp)
    set(WebP_VERSION ${PC_WEBP_VERSION})
    set(WebP_COMPILE_OPTIONS "${PC_WEBP_CFLAGS};${PC_WEBP_CFLAGS_OTHER}")

    pkg_check_modules(PC_WEBP_DECODER QUIET libwebpdecoder)

    pkg_check_modules(PC_WEBP_DEMUX QUIET libwebpdemux)

    pkg_check_modules(PC_WEBP_DECODER QUIET libwebpmux)

    pkg_check_modules(PC_WEBP_SHARPYUV QUIET libsharpyuv)
endif ()

find_path(WebP_INCLUDE_DIR
    NAMES webp/decode.h
    HINTS ${PC_WEBP_INCLUDEDIR} ${PC_WEBP_INCLUDE_DIRS}
)

find_library(WebP_LIBRARY
    NAMES ${WebP_NAMES} webp libwebp
    HINTS ${PC_WEBP_LIBDIR} ${PC_WEBP_LIBRARY_DIRS}
)

# There's nothing in the WebP headers that could be used to detect the exact
# WebP version being used so don't attempt to do so. A version can only be found
# through pkg-config
if (NOT WebP_VERSION)
    message(WARNING "Cannot determine WebP version without pkg-config")
endif ()

if (WebP_INCLUDE_DIR AND WebP_LIBRARY)
    set(WebP_webp_FOUND ON)
else()
    set(WebP_webp_FOUND OFF)
endif()

# Find components
if ("demux" IN_LIST WebP_FIND_COMPONENTS)
    find_library(WebP_DEMUX_LIBRARY
        NAMES ${WebP_DEMUX_NAMES} webpdemux libwebpdemux
        HINTS ${PC_WEBP_LIBDIR} ${PC_WEBP_LIBRARY_DIRS}
    )

    if (WebP_DEMUX_LIBRARY)
        set(WebP_demux_FOUND ON)
    else ()
        set(WebP_demux_FOUND OFF)
    endif ()
endif ()

if ("mux" IN_LIST WebP_FIND_COMPONENTS)
    find_library(WebP_MUX_LIBRARY
        NAMES ${WebP_MUX_NAMES} webpmux libwebpmux
        HINTS ${PC_WEBP_LIBDIR} ${PC_WEBP_LIBRARY_DIRS}
    )

    if (WebP_MUX_LIBRARY)
        set(WebP_mux_FOUND ON)
    else ()
        set(WebP_mux_FOUND OFF)
    endif ()
endif ()

if ("decoder" IN_LIST WebP_FIND_COMPONENTS)
    find_library(WebP_DECODER_LIBRARY
        NAMES ${WebP_DECODER_NAMES} webpdecoder libwebpdecoder
        HINTS ${PC_WEBP_LIBDIR} ${PC_WEBP_LIBRARY_DIRS}
    )

    if (WebP_DECODER_LIBRARY)
        set(WebP_decoder_FOUND ON)
    else ()
        set(WebP_decoder_FOUND OFF)
    endif ()
endif ()

# SharpYUV is a dependency of WebP, should be looked up
# if neither the CMake module nor pkgconf are available
# (this could only happen, in theory, in Windows MSVC)
if ("sharpyuv" IN_LIST WebP_FIND_COMPONENTS OR WebP_webp_FOUND)
    find_library(WebP_SHARPYUV_LIBRARY
        NAMES ${WebP_DECODER_NAMES} sharpyuv libsharpyuv
        HINTS ${PC_WEBP_LIBDIR} ${PC_WEBP_LIBRARY_DIRS}
    )

    if (WebP_SHARPYUV_LIBRARY)
        set(WebP_sharpyuv_FOUND ON)
    else ()
        set(WebP_sharpyuv_FOUND OFF)
    endif ()
endif ()

find_package_handle_standard_args(WebP
    FOUND_VAR WebP_FOUND
    REQUIRED_VARS WebP_INCLUDE_DIR WebP_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR WebP_VERSION
)

if (WebP_FOUND)
if (WebP_LIBRARY AND NOT TARGET WebP::webp)
    add_library(WebP::webp UNKNOWN IMPORTED GLOBAL)
    set_target_properties(WebP::webp PROPERTIES
        IMPORTED_LOCATION "${WebP_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_WEBP_CFLAGS};${PC_WEBP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${WebP_INCLUDE_DIR};${PC_WEBP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PC_WEBP_LINK_LIBRARIES}"
    )
endif ()

if (WebP_DEMUX_LIBRARY AND NOT TARGET WebP::webpdemux)
    add_library(WebP::webpdemux UNKNOWN IMPORTED GLOBAL)
    set_target_properties(WebP::webpdemux PROPERTIES
        IMPORTED_LOCATION "${WebP_DEMUX_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_WEBP_DEMUX_CFLAGS};${PC_WEBP_DEMUX_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${WebP_INCLUDE_DIR};${PC_WEBP_DEMUX_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PC_WEBP_DEMUX_LINK_LIBRARIES}"
    )
endif ()

if (WebP_MUX_LIBRARY AND NOT TARGET WebP::libwebpmux)
    add_library(WebP::libwebpmux UNKNOWN IMPORTED GLOBAL)
    set_target_properties(WebP::libwebpmux PROPERTIES
        IMPORTED_LOCATION "${WebP_MUX_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_WEBP_MUX_CFLAGS};${PC_WEBP_MUX_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${WebP_INCLUDE_DIR};${PC_WEBP_MUX_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PC_WEBP_MUX_LINK_LIBRARIES}"
    )
endif ()

if (WebP_DECODER_LIBRARY AND NOT TARGET WebP::webpdecoder)
    add_library(WebP::webpdecoder UNKNOWN IMPORTED GLOBAL)
    set_target_properties(WebP::webpdecoder PROPERTIES
        IMPORTED_LOCATION "${WebP_DECODER_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_WEBP_DECODER_CFLAGS};${PC_WEBP_DECODER_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${WebP_INCLUDE_DIR};${PC_WEBP_DECODER_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PC_WEBP_DECODER_LINK_LIBRARIES}"
    )
endif ()

if (WebP_SHARPYUV_LIBRARY AND NOT TARGET WebP::sharpyuv)
    add_library(WebP::sharpyuv UNKNOWN IMPORTED GLOBAL)
    set_target_properties(WebP::sharpyuv PROPERTIES
        IMPORTED_LOCATION "${WebP_SHARPYUV_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_WEBP_SHARPYUV_CFLAGS};${PC_WEBP_SHARPYUV_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${WebP_INCLUDE_DIR};${PC_WEBP_SHARPYUV_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PC_WEBP_SHARPYUV_LINK_LIBRARIES}"
    )
endif ()

mark_as_advanced(
    WebP_INCLUDE_DIR
    WebP_LIBRARY
    WebP_DEMUX_LIBRARY
    WebP_MUX_LIBRARY
    WebP_DECODER_LIBRARY
    WebP_SHARPYUV_LIBRARY
)

set(WebP_LIBRARIES ${WebP_LIBRARY} ${WebP_DEMUX_LIBRARY} ${WebP_MUX_LIBRARY} ${WebP_DECODER_LIBRARY} ${WebP_SHARPYUV_LIBRARY})
set(WebP_INCLUDE_DIRS ${WebP_INCLUDE_DIR})

endif()
