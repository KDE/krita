# - Try to find Koffice
# Once done this will define
#
#  KOFFICE_FOUND - system has Koffice
#  KOFFICE_INCLUDE_DIR - the Koffice include directory
#  KOFFICE_LIBRARIES - Link these to use Koffice
#  KOFFICE_DEFINITIONS - Compiler switches required for using Koffice
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if ( KOFFICE_INCLUDE_DIR AND KOFFICE_LIBRARIES )
   # in cache already
   SET(Koffice_FIND_QUIETLY TRUE)
endif ( KOFFICE_INCLUDE_DIR AND KOFFICE_LIBRARIES )

FIND_PATH(KOFFICE_INCLUDE_DIR NAMES KoChild.h
)

FIND_LIBRARY(KOFFICE_LIBRARIES NAMES kotext
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Koffice DEFAULT_MSG KOFFICE_INCLUDE_DIR KOFFICE_LIBRARIES )

# show the KOFFICE_INCLUDE_DIR and KOFFICE_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(KOFFICE_INCLUDE_DIR KOFFICE_LIBRARIES )

