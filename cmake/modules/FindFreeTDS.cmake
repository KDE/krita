# - Try to find the FreeTDS library 
# Once done this will define
#
#  FreeTDS_FOUND - system has FreeTDS
#  FreeTDS_INCLUDE_DIR - the FreeTDS include directory
#  FreeTDS_LIBRARIES - Link these to use FreeTDS

if (FreeTDS_INCLUDE_DIR AND FreeTDS_LIBRARIES)

   # Already in cache
   set(FreeTDS_FOUND TRUE)

else (FreeTDS_INCLUDE_DIR AND FreeTDS_LIBRARIES)

  FIND_PATH(FreeTDS_INCLUDE_DIR NAMES sqldb.h sqlfront.h
    ${_FreeTDSIncDir}
    /usr/include/
    /usr/local/include
  )

  FIND_LIBRARY(FreeTDS_LIBRARIES NAMES tds sybdb
    PATHS
    ${_FreeTDSLinkDir}
    /usr/lib
    /usr/local/lib
  )

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeTDS DEFAULT_MSG FreeTDS_INCLUDE_DIR FreeTDS_LIBRARIES )

  MARK_AS_ADVANCED(FreeTDS_INCLUDE_DIR FreeTDS_LIBRARIES)

endif (FreeTDS_INCLUDE_DIR AND FreeTDS_LIBRARIES)
