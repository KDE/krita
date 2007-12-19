# - Try to find the FreeTDS library 
# Once done this will define
#
#  FreeTDS_FOUND - system has FreeTDS
#  FreeTDS_INCLUDE_DIR - the FreeTDS include directory
#  FreeTDS_LIBRARIES - Link these to use FreeTDS


FIND_PATH(FreeTDS_INCLUDE_DIR NAMES sqldb.h sqlfront.h
    /usr/include/
    /usr/local/include
)

FIND_LIBRARY(FreeTDS_TDS_LIBRARIES NAMES tds )

FIND_LIBRARY(FreeTDS_SYBDB_LIBRARIES NAMES sybdb )
  
set(FreeTDS_LIBRARIES ${FreeTDS_SYBDB_LIBRARIES} ${FreeTDS_TDS_LIBRARIES} CACHE STRING "Libraries needed for sybase/mssql driver")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeTDS DEFAULT_MSG FreeTDS_INCLUDE_DIR FreeTDS_LIBRARIES )

MARK_AS_ADVANCED(FreeTDS_INCLUDE_DIR FreeTDS_LIBRARIES)


