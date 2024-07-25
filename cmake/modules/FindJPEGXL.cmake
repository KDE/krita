# Copyright (C) 2021 Sony Interactive Entertainment Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

#[=======================================================================[.rst:
FindJPEGXL
---------

Find JPEGXL headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``JPEGXL::jxl``
  The JPEGXL library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``JPEGXL_FOUND``
  true if (the requested version of) JPEGXL is available.
``JPEGXL_VERSION``
  the version of JPEGXL.
``JPEGXL_LIBRARIES``
  the libraries to link against to use JPEGXL.
``JPEGXL_INCLUDE_DIRS``
  where to find the JPEGXL headers.
``JPEGXL_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(PC_JPEGXL QUIET libjxl)
    set(JPEGXL_COMPILE_OPTIONS "${PC_JPEGXL_CFLAGS_OTHER}")
    set(JPEGXL_VERSION ${PC_JPEGXL_VERSION})

    pkg_check_modules(PC_JPEGXL_THREADS QUIET libjxl_threads)
    set(JPEGXL_THREADS_COMPILE_OPTIONS ${PC_JPEGXL_THREADS_CFLAGS_OTHER})
endif ()

find_path(JPEGXL_INCLUDE_DIR
    NAMES jxl/decode.h
    HINTS ${PC_JPEGXL_INCLUDEDIR} ${PC_JPEGXL_INCLUDE_DIRS} ${JPEGXL_INCLUDE_DIR}
)

find_library(JPEGXL_LIBRARY
    NAMES ${JPEGXL_NAMES} jxl
    HINTS ${PC_JPEGXL_LIBDIR} ${PC_JPEGXL_LIBRARY_DIRS}
)

find_library(JPEGXL_THREADS_LIBRARY
    NAMES ${JPEGXL_NAMES} jxl_threads
    HINTS ${PC_JPEGXL_LIBDIR} ${PC_JPEGXL_LIBRARY_DIRS} ${PC_JPEGXL_THREADS_LIBDIR} ${PC_JPEGXL_THREADS_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JPEGXL
    FOUND_VAR JPEGXL_FOUND
    REQUIRED_VARS JPEGXL_LIBRARY JPEGXL_INCLUDE_DIR
    VERSION_VAR JPEGXL_VERSION
)

if (JPEGXL_LIBRARY AND NOT TARGET JPEGXL::jxl)
    add_library(JPEGXL::jxl UNKNOWN IMPORTED GLOBAL)
    set_target_properties(JPEGXL::jxl PROPERTIES
        IMPORTED_LOCATION "${JPEGXL_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${JPEGXL_COMPILE_OPTIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${JPEGXL_INCLUDE_DIR}"
    )
endif ()

if (JPEGXL_THREADS_LIBRARY AND NOT TARGET JPEGXL::jxl_threads)
    add_library(JPEGXL::jxl_threads UNKNOWN IMPORTED GLOBAL)
    set_target_properties(JPEGXL::jxl_threads PROPERTIES
        IMPORTED_LOCATION "${JPEGXL_THREADS_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${JPEGXL_THREADS_COMPILE_OPTIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${JPEGXL_INCLUDE_DIR}"
    )
endif ()

mark_as_advanced(JPEGXL_INCLUDE_DIR JPEGXL_LIBRARY JPEGXL_THREADS_LIBRARY)

if (JPEGXL_FOUND)
    set(JPEGXL_LIBRARIES ${JPEGXL_LIBRARY})
    set(JPEGXL_INCLUDE_DIRS ${JPEGXL_INCLUDE_DIR})
endif ()

if (JPEGXL_THREADS_LIBRARY)
    list(APPEND JPEGXL_LIBRARIES ${JPEGXL_THREADS_LIBRARY})
endif ()
