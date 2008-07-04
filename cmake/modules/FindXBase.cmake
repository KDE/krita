# - Try to find the XBase library 
# Once done this will define
#
#  XBase_FOUND - system has XBase
#  XBase_INCLUDE_DIR - the XBase include directory
#  XBase_LIBRARIES - Link these to use XBase


IF (XBase_INCLUDE_DIR AND XBase_LIBRARIES )

	# Already in cache
	SET(XBASE_FOUND TRUE)

ELSE (XBase_INCLUDE_DIR AND XBase_LIBRARIES)

       FIND_PATH(XBase_INCLUDE_DIR NAMES xbase.h
        PATHS /usr/include/xbase /usr/local/include/xbase
       )

       FIND_LIBRARY(XBase_LIBRARIES NAMES xbase )

       include(FindPackageHandleStandardArgs)
       FIND_PACKAGE_HANDLE_STANDARD_ARGS(XBase DEFAULT_MSG XBase_INCLUDE_DIR XBase_LIBRARIES )

       MARK_AS_ADVANCED(XBase_INCLUDE_DIR XBase_LIBRARIES)

ENDIF (XBase_INCLUDE_DIR AND XBase_LIBRARIES )

