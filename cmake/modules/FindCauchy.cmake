# - Try to find Cauchy lib
# Once done this will define
#
#  CAUCHY_FOUND - tell whether cauchy is installed
#  M2MML_FOUND - tell if the M2MML library is installed
#  CAUCHY_INCLUDE_DIR - the cauchy include directory
#  M2MML_LIBRARY - the M2MML library

# Copyright (c) 2011, Cyrille Berger <cberger@cberger.net>
# Redistribution and use is allowed according to the terms of the BSD license.

FIND_PATH(CAUCHY_INCLUDE_DIR m2mml.h
/usr/local/include
/usr/include
)

FIND_LIBRARY(M2MML_LIBRARY
  NAMES m2mml
  PATHS /usr/lib /usr/local/lib )

IF (CAUCHY_INCLUDE_DIR)
  SET(CAUCHY_FOUND "YES")
ELSE ()
  SET(CAUCHY_FOUND "NO")
ENDIF ()

IF(CAUCHY_FOUND AND M2MML_LIBRARY)
  SET(M2MML_FOUND "YES")
ELSE ()
  SET(M2MML_FOUND "NO")
ENDIF ()
