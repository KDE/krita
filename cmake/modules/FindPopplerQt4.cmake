# - Try to find the Qt4 binding of the Poppler library
# Once done this will define
#
#  POPPLER_QT4_FOUND - system has poppler-qt4
#  POPPLER_QT4_INCLUDE_DIR - the poppler-qt4 include directory
#  POPPLER_QT4_LIBRARIES - Link these to use poppler-qt4
#  POPPLER_QT4_DEFINITIONS - Compiler switches required for using poppler-qt4
#

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls

# Copyright (c) 2006, Wilfried Huss, <wilfried.huss@gmx.at>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if(NOT WIN32)
INCLUDE(UsePkgConfig)
  
PKGCONFIG(poppler-qt4 _PopplerQt4IncDir _PopplerQt4LinkDir _PopplerQt4LinkFlags _PopplerQt4Cflags)
  
set(POPPLER_QT4_DEFINITIONS ${_PopplerQt4Cflags})
endif(NOT WIN32)

FIND_PATH(POPPLER_QT4_INCLUDE_DIR poppler-qt4.h
  ${_PopplerQt4IncDir}/poppler
  /usr/include/poppler
  /usr/local/include/poppler
) 
 
FIND_LIBRARY(POPPLER_QT4_LIBRARIES poppler-qt4
  ${_PopplerQt4LinkDir}
  /usr/lib
  /usr/local/lib
)
  
if (POPPLER_QT4_INCLUDE_DIR AND POPPLER_QT4_LIBRARIES)
  set(POPPLER_QT4_FOUND TRUE)
else (POPPLER_QT4_INCLUDE_DIR AND POPPLER_QT4_LIBRARIES)
  set(POPPLER_QT4_FOUND FALSE)
endif (POPPLER_QT4_INCLUDE_DIR AND POPPLER_QT4_LIBRARIES)
  
if (POPPLER_QT4_FOUND)
  if (NOT PopplerQt4_FIND_QUIETLY)
    message(STATUS "Found poppler-qt4: library: ${POPPLER_QT4_LIBRARIES}, include path: ${POPPLER_QT4_INCLUDE_DIR}")
  endif (NOT PopplerQt4_FIND_QUIETLY)
else (POPPLER_QT4_FOUND)
  if (PopplerQt4_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find poppler-qt4")
  endif (PopplerQt4_FIND_REQUIRED)
endif (POPPLER_QT4_FOUND)
  
MARK_AS_ADVANCED(POPPLER_QT4_INCLUDE_DIR POPPLER_QT4_LIBRARIES)
