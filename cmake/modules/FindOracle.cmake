# - Find Oracle / Oracle Embedded
# Find the Oracle includes and client library
# This module defines
#  ORACLE_INCLUDE_DIR, where to find oracle.h
#  ORACLE_LIBRARIES, the libraries needed to use Oracle.
#  ORACLE_EMBEDDED_LIBRARIES, the libraries needed to use Oracle Embedded.
#  ORACLE_FOUND, If false, do not try to use Oracle.
#  ORACLE_EMBEDDED_FOUND, If false, do not try to use Oracle Embedded.

# Copyright (c) 2008, Julia Sanchez-Simon, <hithwen@gmail.com>
# Copyright (c) 2008, Miguel Angel Aragüez-Rey, <fizban87@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


find_path(ORACLE_INCLUDE_DIR nzerror.h occiCommon.h occi.h oci8dp.h ocidef.h ociextp.h 				     ocixmldb.h orid.h oro.h nzt.h occiControl.h occiObjects.h 				     ociap.h ocidem.h oci.h odci.h ori.h ort.h occiAQ.h oci1.h  		             occiData.h  ociapr.h  ocidfn.h  ocikpr.h  oratypes.h  				     orl.h   xa.h

   /usr/include/oracle/11.1.0.1/client
)

find_library(ORACLE_LIBRARIES NAMES occi clntsh
   PATHS
   /usr/lib/oracle/11.1.0.1/client/lib
   /usr/lib/oracle/11.1.0.1/client/bin
)


macro_push_required_vars()
set( CMAKE_REQUIRED_INCLUDES ${ORACLE_INCLUDE_DIR} )
macro_pop_required_vars()

if(ORACLE_INCLUDE_DIR AND ORACLE_LIBRARIES)
   set(ORACLE_FOUND TRUE)
   message(STATUS "Found Oracle: ${ORACLE_INCLUDE_DIR}, ${ORACLE_LIBRARIES}")
else(ORACLE_INCLUDE_DIR AND ORACLE_LIBRARIES)
   set(ORACLE_FOUND FALSE)
   message(STATUS "Oracle not found.")
endif(ORACLE_INCLUDE_DIR AND ORACLE_LIBRARIES)

mark_as_advanced(ORACLE_INCLUDE_DIR ORACLE_LIBRARIES)
