# - Try to find LibWpg
# Once done this will define
#
#  LIBWPG_FOUND       - libwpg is available
#  LIBWPG_INCLUDE_DIR - include directory, e.g. /usr/include
#  LIBWPG_LIBRARIES   - the libraries needed to use LibWpg
#  LIBWPG_DEFINITIONS - Compiler switches required for using LibWpg
#
# Copyright (C) 2007 Ariya Hidayat <ariya@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.

IF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)

  # Already in cache
  set(WPD_FOUND TRUE)
  
ELSE (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)

  IF (NOT WIN32)
    INCLUDE(UsePkgConfig)
    PKGCONFIG(libwpg-0.1 _LibWpgIncDir _LibWpgLinkDir _LibWpgLinkFlags _LibWpgCflags)
    SET(LIBWPG_DEFINITIONS ${_LibWpgCflags})
    ENDIF (NOT WIN32)
  
  FIND_PATH(LIBWPG_INCLUDE_DIR libwpg-0.1/libwpg/libwpg.h
    PATHS
    ${_LibWpgIncDir}
    PATH_SUFFIXES libwpg
    )

  FIND_LIBRARY(LIBWPG_LIBRARIES NAMES wpg-0.1 libwpg-0.1
    PATHS
    ${_LibWpgLinkDir}
    )

  FIND_LIBRARY(LIBWPG_STREAM_LIBRARIES NAMES wpg-stream-0.1 libwpg-stream-0.1
    PATHS
    ${_LibWpgLinkDir}
    )
  
  IF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)
    SET(LIBWPG_FOUND TRUE)
  ELSE (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)
    SET(LIBWPG_FOUND FALSE)
  ENDIF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)
  
  IF (LIBWPG_FOUND)
    MESSAGE(STATUS "Found libwpg: ${LIBWPG_LIBRARIES}")
    MESSAGE("libwpg found " ${LIBWPG_FOUND})
    MESSAGE("libwpg include dir " ${LIBWPG_INCLUDE_DIR})
    MESSAGE("libwpg lib dir " ${_LibWpgLinkDir})
    MESSAGE("libwpg library " ${LIBWPG_LIBRARIES})
    MESSAGE("libwpg stream library " ${LIBWPG_STREAM_LIBRARIES})
    MESSAGE("libwpg cflags " ${LIBWPG_DEFINITIONS})
  ELSE (LIBWPG_FOUND)
    IF (LibWpg_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find libwpg")
    ENDIF (LibWpg_FIND_REQUIRED)
  ENDIF (LIBWPG_FOUND)


ENDIF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)
