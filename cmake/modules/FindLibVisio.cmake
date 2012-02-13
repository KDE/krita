# - Try to find LibVisio
# Once done this will define
#
#  LIBVISIO_FOUND       - libvisio is available
#  LIBVISIO_INCLUDE_DIR - include directory, e.g. /usr/include
#  LIBVISIO_LIBRARIES   - the libraries needed to use LibVisio
#  LIBVISIO_DEFINITIONS - Compiler switches required for using LibVisio
#
# Copyright (C) 2011 Yue Liu <yue.liu@mail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

IF (LIBVISIO_INCLUDE_DIR AND LIBVISIO_LIBRARIES)

  # Already in cache
  set(LIBVISIO_FOUND TRUE)
  
ELSE (LIBVISIO_INCLUDE_DIR AND LIBVISIO_LIBRARIES)

  IF (NOT WIN32)
    INCLUDE(FindPkgConfig)
    pkg_check_modules(LIBVISIO libvisio-0.0)
  ENDIF (NOT WIN32)

  FIND_LIBRARY(LIBVISIO_LIBRARIES
    NAMES
        libvisio-0.0 libwpg-0.2 wpg-0.2 libwpd-0.9 libwpd-stream-0.9 wpd-0.9 wpd-stream-0.9
    PATHS
	${LIBVISIO_LIBRARIES}
  )

  FIND_PATH(LIBVISIO_INCLUDE_DIR libvisio/libvisio.h
    HINTS
        ${LIBWPG_INCLUDE_DIR} ${WPD_INCLUDE_DIR}
    PATH_SUFFIXES 
	libvisio-0.0
  )

  IF (LIBVISIO_INCLUDE_DIR AND LIBVISIO_LIBRARIES)
    SET(LIBVISIO_FOUND TRUE)
  ELSE (LIBVISIO_INCLUDE_DIR AND LIBVISIO_LIBRARIES)
    SET(LIBVISIO_FOUND FALSE)
  ENDIF (LIBVISIO_INCLUDE_DIR AND LIBVISIO_LIBRARIES)
  
  IF (LIBVISIO_FOUND)
    MESSAGE(STATUS "Found libvisio: ${LIBVISIO_LIBRARIES}")
    MESSAGE("libvisio found " ${LIBVISIO_FOUND})
    MESSAGE("libvisio include dir " ${LIBVISIO_INCLUDE_DIR})
    MESSAGE("libvisio lib dir " ${LIBVISIO_LIBRARY_DIRS})
    MESSAGE("libvisio library " ${LIBVISIO_LIBRARIES})
    MESSAGE("libvisio cflags " ${LIBVISIO_DEFINITIONS})
  ELSE (LIBVISIO_FOUND)
    IF (LIBVISIO_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find libvisio")
    ENDIF (LIBVISIO_FIND_REQUIRED)
  ENDIF (LIBVISIO_FOUND)


ENDIF (LIBVISIO_INCLUDE_DIR AND LIBVISIO_LIBRARIES)
