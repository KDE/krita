 # - Try to find the Marble Library
# Once done this will define
#
#  MARBLE_FOUND - system has Marble
#  MARBLE_INCLUDE_DIR - the Marble include directory
#  MARBLE_LIBRARIES 
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if ( MARBLE_INCLUDE_DIR AND MARBLE_LIBRARIES )
   # in cache already
   SET( MARBLE_FIND_QUIETLY TRUE )
endif ( MARBLE_INCLUDE_DIR AND MARBLE_LIBRARIES )

FIND_PATH( MARBLE_INCLUDE_DIR NAMES marble/MarbleMap.h PATH_SUFFIXES marble
)

FIND_LIBRARY( MARBLE_LIBRARIES NAMES marblewidget )


include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( marble DEFAULT_MSG MARBLE_INCLUDE_DIR MARBLE_LIBRARIES )



