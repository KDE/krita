# - Try to find eigen lib
# Once done this will define
#
#  EIGEN_FOUND - system has eigen lib
#  EIGEN_INCLUDE_DIR - the eigen include directory
#
# Copyright (c) 2006, Montel Laurent, <montel@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (EIGEN_INCLUDE_DIR)

  # in cache already
  set(EIGEN_FOUND TRUE)

else (EIGEN_INCLUDE_DIR)

find_path(EIGEN_INCLUDE_DIR NAMES eigen/matrix.h
     PATHS
     ${INCLUDE_INSTALL_DIR}
     /usr/include
     /usr/local/include
   )

if(EIGEN_INCLUDE_DIR)
  set(EIGEN_FOUND TRUE)
endif(EIGEN_INCLUDE_DIR)

if(EIGEN_FOUND)
   if(NOT Eigen_FIND_QUIETLY)
      message(STATUS "Found Eigen: ${EIGEN_INCLUDE_DIR}")
   endif(NOT Eigen_FIND_QUIETLY)
else(EIGEN_FOUND)
   if(Eigen_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find Eigen")
   endif(Eigen_FIND_REQUIRED)
endif(EIGEN_FOUND)

mark_as_advanced(EIGEN_INCLUDE_DIR)

endif(EIGEN_INCLUDE_DIR)

