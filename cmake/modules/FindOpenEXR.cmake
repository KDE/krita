# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
# SPDX-FileCopyrightText: 2013-2014 Alex Merry <alex.merry@kdemail.net>
# SPDX-FileCopyrightText: 2006 Alexander Neundorf <neundorf@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindOpenEXR
-----------

Try to find the OpenEXR libraries.

This will define the following variables:

``OpenEXR_FOUND``
    True if OpenEXR is available
``OpenEXR_LIBRARIES``
    Link to these to use OpenEXR
``OpenEXR_INCLUDE_DIRS``
    Include directory for OpenEXR
``OpenEXR_DEFINITIONS``
    Compiler flags required to link against OpenEXR

and the following imported targets:

``OpenEXR::IlmImf``
    The OpenEXR core library

However, if OpenEXR 3 or higher is found, this target will instead be an
alias to:

``OpenEXR::OpenEXR``
    The OpenEXR version 3 core library

If OpenEXR's Config module is found, this Find module will defer to it.

In general we recommend using the imported target, as it is easier to use.
Bear in mind, however, that if the target is in the link interface of an
exported library, it must be made available by the package config file.

For reference on how to migrate from OpenEXR 2 to 3, which splits the Imath 
library, please see "OpenEXR/Imath 2.x to 3.x Porting Guide" at the Imath 
repository on [GitHub](https://github.com/AcademySoftwareFoundation/Imath/blob/master/docs/PortingGuide2-3.md).

Since pre-1.0.0.
#]=======================================================================]

include(ECMFindModuleHelpersStub)

ecm_find_package_version_check(OpenEXR)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig QUIET)
pkg_check_modules(PC_OpenEXR QUIET OpenEXR)

# Attempt to load OpenEXR's Find module
foreach(_dir ${CMAKE_PREFIX_PATH})
    list(APPEND PC_OpenEXR_CONFIG_DIR ${_dir}/lib/cmake/OpenEXR)
endforeach()
mark_as_advanced(PC_OpenEXR_CONFIG_DIR)

# If there is a CMake config, use it.
find_package(OpenEXR QUIET NO_MODULE
    HINTS ${PC_OpenEXR_CONFIG_DIR} /usr/lib/cmake/OpenEXR /usr/local/lib/cmake/OpenEXR
)

if(OpenEXR_FOUND)
   include(FindPackageHandleStandardArgs)
   find_package_handle_standard_args(OpenEXR HANDLE_COMPONENTS CONFIG_MODE)

   if (OpenEXR_VERSION VERSION_GREATER "3.0.0")
      add_library(OpenEXR::IlmImf ALIAS OpenEXR::OpenEXR)
   endif()
else()
   # Attempt a manual lookup.

   pkg_get_variable(PC_OpenEXR_LIBSUFFIX OpenEXR "libsuffix")

   set(OpenEXR_DEFINITIONS ${PC_OpenEXR_CFLAGS_OTHER})

   find_path(OpenEXR_INCLUDE_DIR ImfRgbaFile.h
      PATHS
      ${PC_OpenEXR_INCLUDEDIR}
      ${PC_OpenEXR_INCLUDE_DIRS}
      PATH_SUFFIXES OpenEXR
   )

   if (OpenEXR_INCLUDE_DIR AND EXISTS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h")
      file(STRINGS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h" openexr_version_str
         REGEX "^#define[\t ]+OPENEXR_VERSION_STRING[\t ]+\"[^\"]*\"")
      string(REGEX REPLACE "^#define[\t ]+OPENEXR_VERSION_STRING[\t ]+\"([^\"]*).*"
            "\\1" OpenEXR_VERSION_STRING "${openexr_version_str}")
      unset(openexr_version_str)

      # These must be parsed manually in case we are in Windows
      # and have no access to pkg-config.

      file(STRINGS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h" openexr_version_str
         REGEX "^#define[\t ]+OPENEXR_VERSION_MAJOR[\t ]+\"[^\"]*\"")
      string(REGEX REPLACE "^#define[\t ]+OPENEXR_VERSION_MAJOR[\t ]+\"([^\"]*).*"
            "\\1" OpenEXR_VERSION_MAJOR "${openexr_version_str}")
      unset(openexr_version_str)

      file(STRINGS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h" openexr_version_str
         REGEX "^#define[\t ]+OPENEXR_VERSION_MINOR[\t ]+\"[^\"]*\"")
      string(REGEX REPLACE "^#define[\t ]+OPENEXR_VERSION_MINOR[\t ]+\"([^\"]*).*"
            "\\1" OpenEXR_VERSION_MINOR "${openexr_version_str}")
      unset(openexr_version_str)

      file(STRINGS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h" openexr_version_str
         REGEX "^#define[\t ]+OPENEXR_VERSION_PATCH[\t ]+\"[^\"]*\"")
      string(REGEX REPLACE "^#define[\t ]+OPENEXR_VERSION_PATCH[\t ]+\"([^\"]*).*"
            "\\1" OpenEXR_VERSION_PATCH "${openexr_version_str}")
      unset(openexr_version_str)
   endif()

   if(OpenEXR_VERSION_STRING VERSION_LESS "3.0.0")
      find_library(OpenEXR_HALF_LIBRARY
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            Half-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}
            Half-${PC_OpenEXR_LIBSUFFIX}
            Half
      )
      find_library(OpenEXR_IEX_LIBRARY
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            Iex-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}
            Iex-${PC_OpenEXR_LIBSUFFIX}
            Iex
      )
      find_library(OpenEXR_IEXMATH_LIBRARY
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            IexMath-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}
            IexMath-${PC_OpenEXR_LIBSUFFIX}
            IexMath
      )
      find_library(OpenEXR_IMATH_LIBRARY
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            Imath-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}
            Imath-${PC_OpenEXR_LIBSUFFIX}
            Imath
      )
      find_library(OpenEXR_ILMTHREAD_LIBRARY
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            IlmThread-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}
            IlmThread-${PC_OpenEXR_LIBSUFFIX}
            IlmThread
      )
      # 2.2.0+ 
      find_library(OpenEXR_ILMIMFUTIL_LIBRARY
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            IlmImfUtil-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}
            IlmImfUtilx-${PC_OpenEXR_LIBSUFFIX}
            IlmImfUtil
      )
      # This is the actual OpenEXR library
      find_library(OpenEXR_ILMIMF_LIBRARY
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            IlmImf-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}
            IlmImf-${PC_OpenEXR_LIBSUFFIX}
            IlmImf
      )

      set(_OpenEXR_deps
         ${OpenEXR_HALF_LIBRARY}
         ${OpenEXR_IEX_LIBRARY}
         ${OpenEXR_IEXMATH_LIBRARY}
         ${OpenEXR_IMATH_LIBRARY}
         ${OpenEXR_ILMTHREAD_LIBRARY}
         ${OpenEXR_ILMIMFUTIL_LIBRARY})

      set(OpenEXR_LIBRARIES
         ${_OpenEXR_deps}
         ${OpenEXR_ILMIMF_LIBRARY})

      # find_package_handle_standard_args reports the value of the first variable
      # on success, so make sure this is the actual OpenEXR library
      find_package_handle_standard_args(OpenEXR
         FOUND_VAR OpenEXR_FOUND
         REQUIRED_VARS
            OpenEXR_ILMIMF_LIBRARY
            OpenEXR_HALF_LIBRARY
            OpenEXR_IEX_LIBRARY
            OpenEXR_IEXMATH_LIBRARY
            OpenEXR_IMATH_LIBRARY
            OpenEXR_ILMTHREAD_LIBRARY
            OpenEXR_ILMIMFUTIL_LIBRARY
            OpenEXR_INCLUDE_DIR
         VERSION_VAR OpenEXR_VERSION_STRING)

      set(OpenEXR_INCLUDE_DIRS ${OpenEXR_INCLUDE_DIR})

      mark_as_advanced(
         OpenEXR_INCLUDE_DIR
         OpenEXR_LIBRARIES
         OpenEXR_DEFINITIONS
         OpenEXR_ILMIMF_LIBRARY
         OpenEXR_ILMIMFUTIL_LIBRARY
         OpenEXR_ILMTHREAD_LIBRARY
         OpenEXR_IMATH_LIBRARY
         OpenEXR_IEX_LIBRARY
         OpenEXR_IEXMATH_LIBRARY
         OpenEXR_HALF_LIBRARY
      )

      if(OpenEXR_FOUND AND NOT TARGET OpenEXR::IlmImf)
         add_library(OpenEXR::IlmImf UNKNOWN IMPORTED)
         set_target_properties(OpenEXR::IlmImf PROPERTIES
            IMPORTED_LOCATION "${OpenEXR_ILMIMF_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${OpenEXR_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenEXR_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${_OpenEXR_deps}"
         )
      endif()
   else()
      # Attempt a manual lookup of Imath.

      pkg_check_modules(PC_Imath QUIET Imath)

      pkg_get_variable(PC_Imath_LIBSUFFIX Imath "libsuffix")

      set(Imath_DEFINITIONS ${PC_Imath_CFLAGS_OTHER})

      find_path(Imath_INCLUDE_DIR ImfConfig.h
         PATHS
         ${PC_Imath_INCLUDEDIR}
         ${PC_Imath_INCLUDE_DIRS}
         PATH_SUFFIXES Imath
      )

      if (Imath_INCLUDE_DIR AND EXISTS "${Imath_INCLUDE_DIR}/ImfConfig.h")
         file(STRINGS "${Imath_INCLUDE_DIR}/ImfConfig.h" imath_version_str
            REGEX "^#define[\t ]+IMATH_VERSION_STRING[\t ]+\"[^\"]*\"")
         string(REGEX REPLACE "^#define[\t ]+IMATH_VERSION_STRING[\t ]+\"([^\"]*).*"
               "\\1" Imath_VERSION_STRING "${imath_version_str}")
         unset(imath_version_str)

         # These must be parsed manually in case we are in Windows
         # and have no access to pkg-config.

         file(STRINGS "${Imath_INCLUDE_DIR}/ImfConfig.h" imath_version_str
            REGEX "^#define[\t ]+IMATH_VERSION_MAJOR[\t ]+\"[^\"]*\"")
         string(REGEX REPLACE "^#define[\t ]+IMATH_VERSION_MAJOR[\t ]+\"([^\"]*).*"
               "\\1" Imath_VERSION_MAJOR "${imath_version_str}")
         unset(imath_version_str)

         file(STRINGS "${Imath_INCLUDE_DIR}/ImfConfig.h" imath_version_str
            REGEX "^#define[\t ]+IMATH_VERSION_MINOR[\t ]+\"[^\"]*\"")
         string(REGEX REPLACE "^#define[\t ]+IMATH_VERSION_MINOR[\t ]+\"([^\"]*).*"
               "\\1" Imath_VERSION_MINOR "${imath_version_str}")
         unset(imath_version_str)

         file(STRINGS "${Imath_INCLUDE_DIR}/ImfConfig.h" imath_version_str
            REGEX "^#define[\t ]+IMATH_VERSION_PATCH[\t ]+\"[^\"]*\"")
         string(REGEX REPLACE "^#define[\t ]+IMATH_VERSION_MAJOR[\t ]+\"([^\"]*).*"
               "\\1" Imath_VERSION_PATCH "${imath_version_str}")
         unset(imath_version_str)
      endif()

      find_library(Imath_LIBRARY
         PATHS
         ${PC_Imath_LIBDIR}
         ${PC_Imath_LIBRARY_DIRS}
         NAMES
            Imath-${Imath_VERSION_MAJOR}_${Imath_VERSION_MINOR}
            Imath-${PC_Imath_LIBSUFFIX}
            Imath
      )
      find_library(OpenEXR_IEX_LIBRARY NAMES Iex
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            Iex-${Imath_VERSION_MAJOR}_${Imath_VERSION_MINOR}
            Iex-${PC_Imath_LIBSUFFIX}
            Iex
      )
      find_library(OpenEXR_ILMTHREAD_LIBRARY NAMES IlmThread
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            IlmThread-${Imath_VERSION_MAJOR}_${Imath_VERSION_MINOR}
            IlmThread-${PC_Imath_LIBSUFFIX}
            IlmThread
      )
      find_library(OpenEXR_OpenEXRUTIL_LIBRARY NAMES OpenEXRUtil
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            OpenEXRUtil-${Imath_VERSION_MAJOR}_${Imath_VERSION_MINOR}
            OpenEXRUtil-${PC_Imath_LIBSUFFIX}
            OpenEXRUtil
      )
      find_library(OpenEXR_OpenEXRCORE_LIBRARY NAMES OpenEXRCore
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            OpenEXRCore-${Imath_VERSION_MAJOR}_${Imath_VERSION_MINOR}
            OpenEXRCore-${PC_Imath_LIBSUFFIX}
            OpenEXRCore
      )
      # This is the actual OpenEXR library
      find_library(OpenEXR_OPENEXR_LIBRARY NAMES OpenEXR
         PATHS
         ${PC_OpenEXR_LIBDIR}
         ${PC_OpenEXR_LIBRARY_DIRS}
         NAMES
            OpenEXR-${Imath_VERSION_MAJOR}_${Imath_VERSION_MINOR}
            OpenEXR-${PC_Imath_LIBSUFFIX}
            OpenEXR
      )

      set(_OpenEXR_deps
         ${Imath_LIBRARY}
         ${OpenEXR_IEX_LIBRARY}
         ${OpenEXR_ILMTHREAD_LIBRARY}
         ${OpenEXR_OpenEXRUTIL_LIBRARY}
         ${OpenEXR_OpenEXRCORE_LIBRARY})

      set(OpenEXR_LIBRARIES
         ${_OpenEXR_deps}
         ${OpenEXR_OPENEXR_LIBRARY})

      # find_package_handle_standard_args reports the value of the first variable
      # on success, so make sure this is the actual OpenEXR library
      find_package_handle_standard_args(OpenEXR
         FOUND_VAR OpenEXR_FOUND
         REQUIRED_VARS
            Imath_LIBRARY
            OpenEXR_IEX_LIBRARY
            OpenEXR_ILMTHREAD_LIBRARY
            OpenEXR_OpenEXRUTIL_LIBRARY
            OpenEXR_OpenEXRCORE_LIBRARY
            OpenEXR_OPENEXR_LIBRARY
            OpenEXR_INCLUDE_DIR
         VERSION_VAR OpenEXR_VERSION_STRING)

      set(OpenEXR_INCLUDE_DIRS ${OpenEXR_INCLUDE_DIR} ${Imath_INCLUDE_DIR})

      mark_as_advanced(
         Imath_INCLUDE_DIR
         OpenEXR_INCLUDE_DIR
         Imath_LIBRARY
         OpenEXR_IEX_LIBRARY
         OpenEXR_ILMTHREAD_LIBRARY
         OpenEXR_OpenEXRUTIL_LIBRARY
         OpenEXR_OpenEXRCORE_LIBRARY
         OpenEXR_OPENEXR_LIBRARY
      )

      if(OpenEXR_FOUND AND NOT TARGET OpenEXR::OpenEXR)
         add_library(OpenEXR::OpenEXR UNKNOWN IMPORTED)
         set_target_properties(OpenEXR::OpenEXR PROPERTIES
            IMPORTED_LOCATION "${OpenEXR_OPENEXR_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${OpenEXR_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenEXR_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${_OpenEXR_deps}"
         )
         add_library(OpenEXR::IlmImf ALIAS OpenEXR::OpenEXR)
      endif()
   endif()
endif()

include(FeatureSummary)
set_package_properties(OpenEXR PROPERTIES
   URL https://www.openexr.com/
   DESCRIPTION "A library for handling OpenEXR high dynamic-range image files")
