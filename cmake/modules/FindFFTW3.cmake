# SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause
#
#[=======================================================================[.rst:
FindFFTW3
--------------

Find FFTW headers and libraries. There are separate targets for each supported
precision, see below.

Imported Targets
^^^^^^^^^^^^^^^^

``FFTW3::fftw3``
  The FFTW3 double precision library, if found.

``FFTW3::fftw3f``
  The FFTW3 single precision library, if found.

``FFTW3::fftw3l``
  The FFTW3 long-double precision library, if found.

``FFTW3::fftw3q``
  The FFTW3 quadruple precision library, if found.

``FFTW3::fftw3_threads``
  The threads support library for the FFTW3 double precision library, if found.

``FFTW3::fftw3f_threads``
  The threads support library for the FFTW3 single precision library, if found.

``FFTW3::fftw3l_threads``
  The threads support library for the FFTW3 long-double precision library, if found.

``FFTW3::fftw3q_threads``
  The threads support library for the FFTW3 quadruple precision library, if found.

``FFTW3::fftw3_openmp``
  The OpenMP support library for the FFTW3 double precision library, if found.

``FFTW3::fftw3f_openmp``
  The OpenMP support library for the FFTW3 single precision library, if found.

``FFTW3::fftw3l_openmp``
  The OpenMP support library for the FFTW3 long-double precision library, if found.

``FFTW3::fftw3q_openmp``
  The OpenMP support library for the FFTW3 quadruple precision library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``FFTW3_FOUND``
  true if (the requested version of) FFTW3 is available.
``FFTW3_VERSION``
  the version of FFTW3.
``FFTW3_LIBRARIES``
  the libraries to link against to use FFTW3.
``FFTW3_INCLUDE_DIRS``
  where to find the FFTW3 headers.
``FFTW3_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]

include(FindPackageHandleStandardArgs)

set(PKG_FFTW_CONFIG_DIR CACHE STRING "PkgConfig path for locating the package modules")
foreach(_dir ${CMAKE_PREFIX_PATH})
    list(APPEND PKG_FFTW_CONFIG_DIR ${_dir}/lib/cmake/fftw3)
endforeach()
mark_as_advanced(PKG_FFTW_CONFIG_DIR)

find_package(FFTW3 QUIET NO_MODULE
    HINTS ${PKG_FFTW_CONFIG_DIR} /usr/lib/cmake/fftw3 /usr/local/lib/cmake/fftw3
)
mark_as_advanced(FFTW3_DIR)

# if we found the FFTW3 CMake package then we can start
# looking up the various precision versions.
# they're exported separately, see e.g. 
# https://archlinux.org/packages/extra/x86_64/fftw/
# https://github.com/microsoft/vcpkg/blob/a325228200d7f229f3337e612e0077f2a5307090/ports/fftw3/portfile.cmake
# https://github.com/Homebrew/homebrew-core/blob/43c39073420de6975f20ac96e98dc822338a3df3/Formula/fftw.rb
if(FFTW3_FOUND)
    find_package(FFTW3f QUIET NO_MODULE
        HINTS ${PKG_FFTW_CONFIG_DIR} /usr/lib/cmake/fftw3 /usr/local/lib/cmake/fftw3
    )
    mark_as_advanced(FFTW3f_DIR)

    find_package(FFTW3l QUIET NO_MODULE
        HINTS ${PKG_FFTW_CONFIG_DIR} /usr/lib/cmake/fftw3 /usr/local/lib/cmake/fftw3
    )
    mark_as_advanced(FFTW3l_DIR)

    find_package(FFTW3q QUIET NO_MODULE
        HINTS ${PKG_FFTW_CONFIG_DIR} /usr/lib/cmake/fftw3 /usr/local/lib/cmake/fftw3
    )
    mark_as_advanced(FFTW3q_DIR)

    # As of fftw3 3.3.10, the precision targets are not
    # available because the library doesn't implement
    # exporting properly; FFTW3LibraryDepends.cmake
    # is fixed to the last built precision.
    # Additionally, the OMP and threads libraries
    # do not have targets altogether.
    #
    # So reconstruct the targets manually.

    # if (TARGET FFTW3::fftw3f)
    #     set(FFTW3_fftw3f_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3f_FOUND OFF)
    # endif ()

    # if (TARGET FFTW3::fftw3l)
    #     set(FFTW3_fftw3l_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3l_FOUND OFF)
    # endif ()

    # if (TARGET FFTW3::fftw3q)
    #     set(FFTW3_fftw3q_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3q_FOUND OFF)
    # endif ()

    # if (TARGET FFTW3::fftw3f_omp)
    #     set(FFTW3_fftw3f_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3f_FOUND OFF)
    # endif ()

    # if (TARGET FFTW3::fftw3l_omp)
    #     set(FFTW3_fftw3l_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3l_FOUND OFF)
    # endif ()

    # if (TARGET FFTW3::fftw3q_omp)
    #     set(FFTW3_fftw3q_omp_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3q_omp_FOUND OFF)
    # endif ()

    # if (TARGET FFTW3::fftw3f_threads)
    #     set(FFTW3_fftw3f_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3f_FOUND OFF)
    # endif ()

    # if (TARGET FFTW3::fftw3l_threads)
    #     set(FFTW3_fftw3l_threads_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3l_threads_FOUND OFF)
    # endif ()

    # if (TARGET FFTW3::fftw3q_threads)
    #     set(FFTW3_fftw3q_threads_FOUND ON)
    # else ()
    #     set(FFTW3_fftw3q_threads_FOUND OFF)
    # endif ()

    # find_package_handle_standard_args(FFTW3 
    #     FOUND_VAR FFTW3_FOUND
    #     HANDLE_COMPONENTS
    #     CONFIG_MODE
    # )
    # return()
endif()

find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(PKG_FFTW QUIET fftw3)
    # Reusing information from the existing CMake package file, if any.
    if (NOT FFTW3_VERSION)
        set(FFTW3_VERSION ${PKG_FFTW_VERSION})
    endif()
    set(FFTW3_COMPILE_OPTIONS "${PKG_FFTW_CFLAGS};${PKG_FFTW_CFLAGS_OTHER}")

    pkg_check_modules(PKG_FFTW_OMP QUIET fftw3_omp)
    pkg_check_modules(PKG_FFTW_THREADS QUIET fftw3_threads)

    pkg_check_modules(PKG_FFTW_SINGLE QUIET fftw3f)
    pkg_check_modules(PKG_FFTW_LONGDOUBLE QUIET fftw3l)
    pkg_check_modules(PKG_FFTW_QUAD QUIET fftw3q)

    pkg_check_modules(PKG_FFTW_SINGLE_OMP QUIET fftw3f_omp)
    pkg_check_modules(PKG_FFTW_LONGDOUBLE_OMP QUIET fftw3l_omp)
    pkg_check_modules(PKG_FFTW_QUAD_OMP QUIET fftw3q_omp)

    pkg_check_modules(PKG_FFTW_SINGLE_THREADS QUIET fftw3f_omp)
    pkg_check_modules(PKG_FFTW_LONGDOUBLE_THREADS QUIET fftw3l_omp)
    pkg_check_modules(PKG_FFTW_QUAD_OMP QUIET fftw3q_omp)
endif()

find_path(FFTW3_INCLUDE_DIR
    NAMES "fftw3.h"
    HINTS ${FFTW3_INCLUDE_DIRS} ${PKG_FFTW_INCLUDEDIR} ${PKG_FFTW_INCLUDE_DIRS}
)

find_library(FFTW3_LIBRARY
    NAMES fftw3 libfftw3
    HINTS ${FFTW3_LIBRARY_DIRS} ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
)

# There's nothing in the FFTW3 headers that could be used to detect the exact
# FFTW3 version being used so don't attempt to do so. A version can only be found
# through pkg-config
if (NOT FFTW3_VERSION)
    message(WARNING "Cannot determine FFTW3 version without pkg-config")
endif ()

if (FFTW3_INCLUDE_DIR AND FFTW3_LIBRARY)
    set(FFTW3_fftw3_FOUND ON)
else()
    set(FFTW3_fftw3_FOUND OFF)
endif()

# Find components
if ("fftw3_omp" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3_omp_LIBRARY
        NAMES fftw3_omp libfftw3_omp
        HINTS
            ${FFTW3_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_OMP_LIBDIR} ${PKG_FFTW_OMP_LIBRARY_DIRS}
    )

    if (FFTW3_fftw3_omp_LIBRARY)
        set(FFTW3_fftw3_omp_FOUND ON)
    else ()
        set(FFTW3_fftw3_omp_FOUND OFF)
    endif ()
endif ()

if ("fftw3_threads" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3_threads_LIBRARY
        NAMES fftw3_threads libfftw3_threads
        HINTS
            ${FFTW3_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_THREADS_LIBDIR} ${PKG_FFTW_THREADS_LIBRARY_DIRS}
    )

    if (FFTW3_fftw3_threads_LIBRARY)
        set(FFTW3_fftw3_threads_FOUND ON)
    else ()
        set(FFTW3_fftw3_threads_FOUND OFF)
    endif ()
endif ()

if ("fftw3f" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3f_LIBRARY
        NAMES fftw3f libfftw3f
        HINTS
            ${FFTW3_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_SINGLE_LIBDIR} ${PKG_FFTW_SINGLE_LIBRARY_DIRS}
    )

    if(FFTW3_fftw3f_LIBRARY)
        set(FFTW3_fftw3f_FOUND ON)
    else()
        set(FFTW3_fftw3f_FOUND OFF)
    endif ()
endif ()

if ("fftw3f_omp" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3f_omp_LIBRARY
        NAMES fftw3f_omp libfftw3f_omp
        HINTS
            ${FFTW3_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_SINGLE_LIBDIR} ${PKG_FFTW_SINGLE_LIBRARY_DIRS}
            ${PKG_FFTW_SINGLE_OMP_LIBDIR} ${PKG_FFTW_SINGLE_OMP_LIBRARY_DIRS}
    )

    if (FFTW3_fftw3f_omp_LIBRARY)
        set(FFTW3_fftw3f_omp_FOUND ON)
    else ()
        set(FFTW3_fftw3f_omp_FOUND OFF)
    endif ()
endif ()

if ("fftw3f_threads" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3f_threads_LIBRARY
        NAMES fftw3f_threads libfftw3f_threads
        HINTS
            ${FFTW3_LIBRARY_DIRS} ${FFTW3f_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_SINGLE_LIBDIR} ${PKG_FFTW_SINGLE_LIBRARY_DIRS}
            ${PKG_FFTW_SINGLE_OMP_LIBDIR} ${PKG_FFTW_SINGLE_OMP_LIBRARY_DIRS}
    )

    if (FFTW3_fftw3f_threads_LIBRARY)
        set(FFTW3_fftw3f_threads_FOUND ON)
    else ()
        set(FFTW3_fftw3f_threads_FOUND OFF)
    endif ()
endif ()

if ("fftw3l" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3l_LIBRARY
        NAMES fftw3l libfftw3l
        HINTS
            ${FFTW3_LIBRARY_DIRS} ${FFTW3l_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_LONGDOUBLE_LIBDIR} ${PKG_FFTW_LONGDOUBLE_LIBRARY_DIRS}
    )

    if(FFTW3_fftw3l_LIBRARY)
        set(FFTW3_fftw3l_FOUND ON)
    else()
        set(FFTW3_fftw3l_FOUND OFF)
    endif ()
endif ()

if ("fftw3l_omp" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3l_omp_LIBRARY
        NAMES fftw3l_omp libfftw3l_omp
        HINTS
            ${FFTW3_LIBRARY_DIRS} ${FFTW3l_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_LONGDOUBLE_LIBDIR} ${PKG_FFTW_LONGDOUBLE_LIBRARY_DIRS}
            ${PKG_FFTW_LONGDOUBLE_OMP_LIBDIR} ${PKG_FFTW_LONGDOUBLE_OMP_LIBRARY_DIRS}
    )

    if (FFTW3_fftw3l_omp_LIBRARY)
        set(FFTW3_fftw3l_omp_FOUND ON)
    else ()
        set(FFTW3_fftw3l_omp_FOUND OFF)
    endif ()
endif ()

if ("fftw3l_threads" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3l_threads_LIBRARY
        NAMES fftw3l_threads libfftw3l_threads
        HINTS
            ${FFTW3_LIBRARY_DIRS} ${FFTW3l_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_LONGDOUBLE_LIBDIR} ${PKG_FFTW_LONGDOUBLE_LIBRARY_DIRS}
            ${PKG_FFTW_LONGDOUBLE_OMP_LIBDIR} ${PKG_FFTW_LONGDOUBLE_OMP_LIBRARY_DIRS}
    )

    if (FFTW3_fftw3l_threads_LIBRARY)
        set(FFTW3_fftw3l_threads_FOUND ON)
    else ()
        set(FFTW3_fftw3l_threads_FOUND OFF)
    endif ()
endif ()

if ("fftw3q" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3q_LIBRARY
        NAMES fftw3q libfftw3q
        HINTS
            ${FFTW3_LIBRARY_DIRS} ${FFTW3q_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_QUAD_LIBDIR} ${PKG_FFTW_QUAD_LIBRARY_DIRS}
    )

    if(FFTW3_fftw3q_LIBRARY)
        set(FFTW3_fftw3q_FOUND ON)
    else()
        set(FFTW3_fftw3q_FOUND OFF)
    endif ()
endif ()

if ("fftw3q_omp" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3q_omp_LIBRARY
        NAMES fftw3q_omp libfftw3q_omp
        HINTS
            ${FFTW3_LIBRARY_DIRS} ${FFTW3q_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_QUAD_LIBDIR} ${PKG_FFTW_QUAD_LIBRARY_DIRS}
            ${PKG_FFTW_QUAD_OMP_LIBDIR} ${PKG_FFTW_QUAD_OMP_LIBRARY_DIRS}
    )

    if (FFTW3_fftw3q_omp_LIBRARY)
        set(FFTW3_fftw3q_omp_FOUND ON)
    else ()
        set(FFTW3_fftw3q_omp_FOUND OFF)
    endif ()
endif ()

if ("fftw3q_threads" IN_LIST FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_fftw3q_threads_LIBRARY
        NAMES fftw3q_threads libfftw3q_threads
        HINTS
            ${FFTW3_LIBRARY_DIRS} ${FFTW3q_LIBRARY_DIRS}
            ${PKG_FFTW_LIBDIR} ${PKG_FFTW_LIBRARY_DIRS}
            ${PKG_FFTW_QUAD_LIBDIR} ${PKG_FFTW_QUAD_LIBRARY_DIRS}
            ${PKG_FFTW_QUAD_OMP_LIBDIR} ${PKG_FFTW_QUAD_OMP_LIBRARY_DIRS}
    )

    if (FFTW3_fftw3q_threads_LIBRARY)
        set(FFTW3_fftw3q_threads_FOUND ON)
    else ()
        set(FFTW3_fftw3q_threads_FOUND OFF)
    endif ()
endif ()

find_package_handle_standard_args(FFTW3
    FOUND_VAR FFTW3_FOUND
    REQUIRED_VARS FFTW3_INCLUDE_DIR FFTW3_LIBRARY
    HANDLE_COMPONENTS
    VERSION_VAR FFTW3_VERSION
)

if (FFTW3_FOUND)
if (FFTW3_LIBRARY AND NOT TARGET FFTW3::fftw3)
    add_library(FFTW3::fftw3 UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3 PROPERTIES
        IMPORTED_LOCATION "${FFTW3_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_CFLAGS};${PKG_FFTW_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3_omp_LIBRARY AND NOT TARGET FFTW3::fftw3_omp)
    add_library(FFTW3::fftw3_omp UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3_omp PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3_omp_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_OMP_CFLAGS};${PKG_FFTW_OMP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_OMP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_OMP_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3_threads_LIBRARY AND NOT TARGET FFTW3::fftw3_threads)
    add_library(FFTW3::fftw3_threads UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3 PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3_threads_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_THREADS_CFLAGS};${PKG_FFTW_THREADS_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_THREADS_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_THREADS_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3f_LIBRARY AND NOT TARGET FFTW3::fftw3f)
    add_library(FFTW3::fftw3f UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3f PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3f_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_SINGLE_CFLAGS};${PKG_FFTW_SINGLE_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_SINGLE_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_SINGLE_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3f_omp_LIBRARY AND NOT TARGET FFTW3::fftw3f_omp)
    add_library(FFTW3::fftw3f_omp UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3f_omp PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3f_omp_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_SINGLE_OMP_CFLAGS};${PKG_FFTW_SINGLE_OMP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_SINGLE_OMP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_SINGLE_OMP_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3f_threads_LIBRARY AND NOT TARGET FFTW3::fftw3f_threads)
    add_library(FFTW3::fftw3f_threads UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3f PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3f_threads_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_THREADS_CFLAGS};${PKG_FFTW_SINGLE_THREADS_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_SINGLE_THREADS_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_SINGLE_THREADS_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3l_LIBRARY AND NOT TARGET FFTW3::fftw3l)
    add_library(FFTW3::fftw3l UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3l PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3l_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_LONGDOUBLE_CFLAGS};${PKG_FFTW_LONGDOUBLE_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_LONGDOUBLE_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_LONGDOUBLE_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3l_omp_LIBRARY AND NOT TARGET FFTW3::fftw3l_omp)
    add_library(FFTW3::fftw3l_omp UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3l_omp PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3l_omp_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_LONGDOUBLE_OMP_CFLAGS};${PKG_FFTW_LONGDOUBLE_OMP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_LONGDOUBLE_OMP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_LONGDOUBLE_OMP_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3l_threads_LIBRARY AND NOT TARGET FFTW3::fftw3l_threads)
    add_library(FFTW3::fftw3l_threads UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3l_threads PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3l_threads_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_THREADS_CFLAGS};${PKG_FFTW_LONGDOUBLE_THREADS_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_LONGDOUBLE_THREADS_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_LONGDOUBLE_THREADS_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3q_LIBRARY AND NOT TARGET FFTW3::fftw3q)
    add_library(FFTW3::fftw3q UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3q PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3q_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_QUAD_CFLAGS};${PKG_FFTW_QUAD_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_QUAD_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_QUAD_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3q_omp_LIBRARY AND NOT TARGET FFTW3::fftw3q_omp)
    add_library(FFTW3::fftw3q_omp UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3q_omp PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3q_omp_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_QUAD_OMP_CFLAGS};${PKG_FFTW_QUAD_OMP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_QUAD_OMP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_QUAD_OMP_LINK_LIBRARIES}"
    )
endif ()

if (FFTW3_fftw3q_THREADS_LIBRARY AND NOT TARGET FFTW3::fftw3q_threads)
    add_library(FFTW3::fftw3q_threads UNKNOWN IMPORTED GLOBAL)
    set_target_properties(FFTW3::fftw3q PROPERTIES
        IMPORTED_LOCATION "${FFTW3_fftw3q_threads_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PKG_FFTW_THREADS_CFLAGS};${PKG_FFTW_QUAD_THREADS_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR};${PKG_FFTW_QUAD_THREADS_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PKG_FFTW_QUAD_THREADS_LINK_LIBRARIES}"
    )
endif ()

mark_as_advanced(
    FFTW3_INCLUDE_DIR
    FFTW3_LIBRARY
    FFTW3_fftw3f_LIBRARY
    FFTW3_fftw3f_OMP_LIBRARY
    FFTW3_fftw3f_THREADS_LIBRARY
    FFTW3_fftw3l_LIBRARY
    FFTW3_fftw3l_OMP_LIBRARY
    FFTW3_fftw3l_THREADS_LIBRARY
    FFTW3_fftw3q_LIBRARY
    FFTW3_fftw3q_OMP_LIBRARY
    FFTW3_fftw3q_THREADS_LIBRARY
)

set(FFTW3_LIBRARIES
    ${FFTW3_LIBRARY}
    ${FFTW3_fftw3f_LIBRARY}
    ${FFTW3_fftw3f_OMP_LIBRARY}
    ${FFTW3_fftw3f_THREADS_LIBRARY}
    ${FFTW3_fftw3l_LIBRARY}
    ${FFTW3_fftw3l_OMP_LIBRARY}
    ${FFTW3_fftw3l_THREADS_LIBRARY}
    ${FFTW3_fftw3q_LIBRARY}
    ${FFTW3_fftw3q_OMP_LIBRARY}
    ${FFTW3_fftw3q_THREADS_LIBRARY}
)
set(FFTW_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
endif ()
