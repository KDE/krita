# - Try to find the Marble Library
# Once done this will define
#
#  MARBLE_FOUND - system has Marble
#  MARBLE_INCLUDE_DIR - the Marble include directory
#  MARBLE_LIBRARIES - Libraries needed to use Marble
#  MARBLE_VERSION - This contains combined MAJOR.MINOR.PATCH version (eg. 0.19.2);
#                   Can be missing if version could not be found
#
#  Versions mapping can be found at: https://marble.kde.org/changelog.php
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
# Copyright 2009-2011 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

function(from_hex HEX DEC)
  string(TOUPPER "${HEX}" HEX)
  set(_res 0)
  string(LENGTH "${HEX}" _strlen)

  while (_strlen GREATER 0)
    math(EXPR _res "${_res} * 16")
    string(SUBSTRING "${HEX}" 0 1 NIBBLE)
    string(SUBSTRING "${HEX}" 1 -1 HEX)
    if (NIBBLE STREQUAL "A")
      math(EXPR _res "${_res} + 10")
    elseif (NIBBLE STREQUAL "B")
      math(EXPR _res "${_res} + 11")
    elseif (NIBBLE STREQUAL "C")
      math(EXPR _res "${_res} + 12")
    elseif (NIBBLE STREQUAL "D")
      math(EXPR _res "${_res} + 13")
    elseif (NIBBLE STREQUAL "E")
      math(EXPR _res "${_res} + 14")
    elseif (NIBBLE STREQUAL "F")
      math(EXPR _res "${_res} + 15")
    else()
      math(EXPR _res "${_res} + ${NIBBLE}")
    endif()

    string(LENGTH "${HEX}" _strlen)
  endwhile()

  set(${DEC} ${_res} PARENT_SCOPE)
endfunction()

if ( MARBLE_INCLUDE_DIR AND MARBLE_GLOBAL_HEADER AND MARBLE_LIBRARIES )
   # in cache already
   set( MARBLE_FIND_QUIETLY TRUE )
endif ()

find_path( MARBLE_INCLUDE_DIR NAMES marble/MarbleMap.h PATH_SUFFIXES marble)
find_file( MARBLE_GLOBAL_HEADER NAMES marble/MarbleGlobal.h PATH_SUFFIXES marble)
find_library( MARBLE_LIBRARIES NAMES marblewidget-qt5 )

if(MARBLE_GLOBAL_HEADER)
    file(STRINGS ${MARBLE_GLOBAL_HEADER}
         marble_version_line
         REGEX "^#define[\t ]+MARBLE_VERSION[\t ]+0x([0-9a-fA-F])+.*")

    string(REGEX REPLACE
            "^.*MARBLE_VERSION[\t ]+0x([0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F]).*$"
            "\\1;\\2;\\3" marble_version_list "${marble_version_line}")

    list(GET marble_version_list 0 MARBLE_VERSION_MAJOR)
    from_hex("${MARBLE_VERSION_MAJOR}" MARBLE_VERSION_MAJOR)

    list(GET marble_version_list 1 MARBLE_VERSION_MINOR)
    from_hex("${MARBLE_VERSION_MINOR}" MARBLE_VERSION_MINOR)

    list(GET marble_version_list 2 MARBLE_VERSION_PATCH)
    from_hex("${MARBLE_VERSION_PATCH}" MARBLE_VERSION_PATCH)

    set(MARBLE_VERSION "${MARBLE_VERSION_MAJOR}.${MARBLE_VERSION_MINOR}.${MARBLE_VERSION_PATCH}" CACHE STRING "Found Marble version")
endif()

include( FindPackageHandleStandardArgs )

if(MARBLE_VERSION)
    if(DEFINED MARBLE_MIN_VERSION AND ${MARBLE_VERSION} VERSION_LESS ${MARBLE_MIN_VERSION})
        set(MARBLE_FOUND FALSE)
        unset(MARBLE_INCLUDE_DIR)
        unset(MARBLE_LIBRARIES)
    else()
        find_package_handle_standard_args( Marble
            REQUIRED_VARS
                MARBLE_INCLUDE_DIR
                MARBLE_LIBRARIES
            VERSION_VAR
                MARBLE_VERSION
            FAIL_MESSAGE
                "Could not find Marble"
        )
    endif()
else()
    find_package_handle_standard_args( marble
            DEFAULT_MSG
            MARBLE_INCLUDE_DIR
            MARBLE_LIBRARIES )
endif()

mark_as_advanced(MARBLE_GLOBAL_HEADER MARBLE_VERSION_MAJOR MARBLE_VERSION_MINOR MARBLE_VERSION_PATCH)
