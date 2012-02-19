# - Try to find LibWPS
# Once done this will define
#
#  LIBWPS_FOUND       - libwps is available
#  LIBWPS_INCLUDE_DIR - include directory, e.g. /usr/include
#  LIBWPS_LIBRARIES   - the libraries needed to use LibWPS
#  LIBWPS_DEFINITIONS - Compiler switches required for using LibWPS
#
# Copyright (C) 2012 Yue Liu <yue.liu@mail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

IF (LIBWPS_INCLUDE_DIR AND LIBWPS_LIBRARIES)

  # Already in cache
  set(LIBWPS_FOUND TRUE)
  
ELSE (LIBWPS_INCLUDE_DIR AND LIBWPS_LIBRARIES)

  IF (NOT WIN32)
    INCLUDE(FindPkgConfig)
    pkg_check_modules(LIBWPS libwps-0.2)
  ENDIF (NOT WIN32)

  FIND_LIBRARY(LIBWPS_LIBRARIES
    NAMES
        libwps-0.2 libwpd-0.9 libwpd-stream-0.9 wpd-0.9 wpd-stream-0.9
    PATHS
	${LIBWPS_LIBRARIES}
  )

  FIND_PATH(LIBWPS_INCLUDE_DIR libwps/libwps.h
    HINTS
        ${WPD_INCLUDE_DIR}
    PATH_SUFFIXES 
	libwps-0.2
  )

  IF (LIBWPS_INCLUDE_DIR AND LIBWPS_LIBRARIES)
    SET(LIBWPS_FOUND TRUE)
  ELSE (LIBWPS_INCLUDE_DIR AND LIBWPS_LIBRARIES)
    SET(LIBWPS_FOUND FALSE)
  ENDIF (LIBWPS_INCLUDE_DIR AND LIBWPS_LIBRARIES)
  
  IF (LIBWPS_FOUND)
    MESSAGE(STATUS "Found libwps: ${LIBWPS_LIBRARIES}")
    MESSAGE("libwps found " ${LIBWPS_FOUND})
    MESSAGE("libwps include dir " ${LIBWPS_INCLUDE_DIR})
    MESSAGE("libwps lib dir " ${LIBWPS_LIBRARY_DIRS})
    MESSAGE("libwps library " ${LIBWPS_LIBRARIES})
    MESSAGE("libwps cflags " ${LIBWPS_DEFINITIONS})
  ELSE (LIBWPS_FOUND)
    IF (LIBWPS_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find libwps")
    ENDIF (LIBWPS_FIND_REQUIRED)
  ENDIF (LIBWPS_FOUND)


ENDIF (LIBWPS_INCLUDE_DIR AND LIBWPS_LIBRARIES)
