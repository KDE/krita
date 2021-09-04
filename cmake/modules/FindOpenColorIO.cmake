# Module to find OpenColorIO
#
# This module will first look into the directories hinted by the variables:
#   - OpenColorIO_ROOT
#
# This module defines the following variables:
#
# OPENCOLORIO_FOUND       - True if OpenColorIO was found.
# OPENCOLORIO_INCLUDES    - where to find OpenColorIO.h
# OPENCOLORIO_LIBRARIES   - list of libraries to link against when using OpenColorIO
# OPENCOLORIO_DEFINITIONS - Definitions needed when using OpenColorIO
#
# SPDX-FileCopyrightText: 2008 Contributors to the OpenImageIO project
# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause

include (FindPackageHandleStandardArgs)
include (FindPackageMessage)

find_path (OPENCOLORIO_INCLUDE_DIR
    OpenColorIO.h
    HINTS
        ${OPENCOLORIO_INCLUDE_PATH}
        ENV OPENCOLORIO_INCLUDE_PATH
    PATHS
        /sw/include
        /opt/local/include
    PATH_SUFFIXES OpenColorIO OpenColorIO1
    DOC "The directory where OpenColorIO.h resides")

if (EXISTS "${OPENCOLORIO_INCLUDE_DIR}/OpenColorABI.h")
    # Search twice, because this symbol changed between OCIO 1.x and 2.x
    file(STRINGS "${OPENCOLORIO_INCLUDE_DIR}/OpenColorABI.h" TMP
         REGEX "^#define OCIO_VERSION_STR[ \t].*$")
    if (NOT TMP)
        file(STRINGS "${OPENCOLORIO_INCLUDE_DIR}/OpenColorABI.h" TMP
             REGEX "^#define OCIO_VERSION[ \t].*$")
    endif ()
    string (REGEX MATCHALL "([0-9]+)\\.([0-9]+)\\.[0-9]+" OPENCOLORIO_VERSION ${TMP})
    set (OPENCOLORIO_VERSION_MAJOR ${CMAKE_MATCH_1})
    set (OPENCOLORIO_VERSION_MINOR ${CMAKE_MATCH_2})
endif ()

find_library (OPENCOLORIO_LIBRARY
    NAMES
        OpenColorIO
        OpenColorIO_${OPENCOLORIO_VERSION_MAJOR}_${OPENCOLORIO_VERSION_MINOR}
        OpenColorIO${OPENCOLORIO_VERSION_MAJOR}
    HINTS
        ${OPENCOLORIO_LIBRARY_PATH}
        ENV OPENCOLORIO_LIBRARY_PATH
    PATHS
        /usr/lib64
        /usr/local/lib64
        /sw/lib
        /opt/local/lib
    DOC "The OCIO library")

find_package_handle_standard_args (OpenColorIO
    REQUIRED_VARS   OPENCOLORIO_INCLUDE_DIR OPENCOLORIO_LIBRARY
    FOUND_VAR       OPENCOLORIO_FOUND
    VERSION_VAR     OPENCOLORIO_VERSION
    )

if (OpenColorIO_FOUND)
    set (OPENCOLORIO_INCLUDES ${OPENCOLORIO_INCLUDE_DIR})
    set (OPENCOLORIO_LIBRARIES ${OPENCOLORIO_LIBRARY})
    set (OPENCOLORIO_DEFINITIONS "")
    if (NOT TARGET OpenColorIO::OpenColorIO)
        add_library(OpenColorIO::OpenColorIO UNKNOWN IMPORTED)
        set_target_properties(OpenColorIO::OpenColorIO PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${OPENCOLORIO_INCLUDES}")

        set_property(TARGET OpenColorIO::OpenColorIO APPEND PROPERTY
            IMPORTED_LOCATION "${OPENCOLORIO_LIBRARIES}")
        if (LINKSTATIC)
            target_compile_definitions(OpenColorIO::OpenColorIO
                INTERFACE "-DOpenColorIO_STATIC")
        endif()
    endif ()
    if (NOT TARGET OpenColorIO::OpenColorIOHeaders)
        add_library(OpenColorIO::OpenColorIOHeaders INTERFACE IMPORTED)
        set_target_properties(OpenColorIO::OpenColorIOHeaders PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${OPENCOLORIO_INCLUDES}")
    endif ()
endif ()

if (OpenColorIO_FOUND AND LINKSTATIC)
    # Is this necessary?
    set (OPENCOLORIO_DEFINITIONS "-DOpenColorIO_STATIC")
    find_library (TINYXML_LIBRARY NAMES tinyxml)
    if (TINYXML_LIBRARY)
        set (OPENCOLORIO_LIBRARIES "${OPENCOLORIO_LIBRARIES};${TINYXML_LIBRARY}" CACHE STRING "" FORCE)
    endif ()
    find_library (YAML_LIBRARY NAMES yaml-cpp)
    if (YAML_LIBRARY)
        set (OPENCOLORIO_LIBRARIES "${OPENCOLORIO_LIBRARIES};${YAML_LIBRARY}" CACHE STRING "" FORCE)
    endif ()
    find_library (LCMS2_LIBRARY NAMES lcms2)
    if (LCMS2_LIBRARY)
        set (OPENCOLORIO_LIBRARIES "${OPENCOLORIO_LIBRARIES};${LCMS2_LIBRARY}" CACHE STRING "" FORCE)
    endif ()
endif ()

