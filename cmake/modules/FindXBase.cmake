# - Try to find the XBase library 
# Once done this will define
#
#  XBase_FOUND - system has XBase
#  XBase_INCLUDE_DIR - the XBase include directory
#  XBase_LIBRARIES - Link these to use XBase


if (XBase_INCLUDE_DIR AND XBase_LIBRARIES )

	# Already in cache
	SET(XBASE_FOUND TRUE)

else ()

       find_path(XBase_INCLUDE_DIR NAMES xbase.h
        PATHS /usr/include/xbase /usr/local/include/xbase
       )

       find_library(XBase_LIBRARIES NAMES xbase )

       include(FindPackageHandleStandardArgs)
       find_package_handle_standard_args(XBase DEFAULT_MSG XBase_INCLUDE_DIR XBase_LIBRARIES )

       mark_as_advanced(XBase_INCLUDE_DIR XBase_LIBRARIES)

endif ()

