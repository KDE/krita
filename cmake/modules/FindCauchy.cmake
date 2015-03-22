# - Try to find Cauchy lib
# Once done this will define
#
#  CAUCHY_FOUND - tell whether cauchy is installed
#  M2MML_FOUND - tell if the M2MML library is installed
#  CAUCHY_INCLUDE_DIR - the cauchy include directory
#  M2MML_LIBRARY - the M2MML library

# Copyright (c) 2011, Cyrille Berger <cberger@cberger.net>
# Redistribution and use is allowed according to the terms of the BSD license.

find_path(CAUCHY_INCLUDE_DIR m2mml.h
/usr/local/include
/usr/include
)

find_library(M2MML_LIBRARY
  NAMES m2mml
  PATHS /usr/lib /usr/local/lib )

if (CAUCHY_INCLUDE_DIR)
  set(CAUCHY_FOUND "YES")
else ()
  set(CAUCHY_FOUND "NO")
endif ()

if(CAUCHY_FOUND AND M2MML_LIBRARY)
  set(M2MML_FOUND "YES")
else ()
  set(M2MML_FOUND "NO")
endif ()
