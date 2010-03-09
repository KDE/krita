# OOOSDK_FOUND
# OOOSDK_INCLUDE_DIRS
# OOOSDK_LIBRARY_DIRS
# OOOSDK_LIBRARIES

# look for cppuhelper/bootstrap.hxx

find_path(CPPUHELPER_INCLUDE_DIR
	NAMES cppuhelper/bootstrap.hxx
	PATHS /usr/include/openoffice
)
if (NOT CPPUHELPER_INCLUDE_DIR)
	set(OOOSDK_ERROR 
		"Could not find cppuhelper/bootstrap.hxx for OOoSDK.")
endif (NOT CPPUHELPER_INCLUDE_DIR)

find_path(OOOSDK_DIR
	NAMES sdk/bin/cppumaker program/offapi.rdb
	PATHS /opt/openoffice.org/basis3.2
	      /usr/lib/openoffice/basis3.1
	      /usr/lib/openoffice/basis3.0
)
if (NOT OOOSDK_DIR)
	set(OOOSDK_ERROR "Could not find sdk/bin/cppumaker
		or program/offapi.rdb for OOoSDK.")
endif (NOT OOOSDK_DIR)

find_path(OOOSDK_URE_DIR
	NAMES share/misc/types.rdb 
	PATHS /opt/openoffice.org/ure
	      /usr/lib/ure
)
if (NOT OOOSDK_URE_DIR)
	set(OOOSDK_ERROR "Could not find share/misc/types.rdb for OOoSDK.")
endif (NOT OOOSDK_URE_DIR)

find_library(OOOSDK_LIBRARIES
	NAMES uno_cppuhelpergcc3
	PATHS ${OOOSDK_DIR}/sdk/lib
	      /usr/lib/ure/lib
)
if (NOT OOOSDK_LIBRARIES)
	set(OOOSDK_ERROR "Could not find uno_cppuhelpergcc3 for OOoSDK.")
endif (NOT OOOSDK_LIBRARIES)

if(CPPUHELPER_INCLUDE_DIR AND OOOSDK_DIR AND OOOSDK_URE_DIR AND OOOSDK_LIBRARIES)
	set(OOOSDK_FOUND true)

	# generate c++ headers
	set(_OOOINCLUDE ${CMAKE_CURRENT_BINARY_DIR}/ooosdkinclude)
	file(MAKE_DIRECTORY ${_OOOINCLUDE})
	add_custom_command(
		OUTPUT ${_OOOINCLUDE}/com/sun/star/uno/Exception.hpp
		COMMAND ${OOOSDK_DIR}/sdk/bin/cppumaker
		ARGS -BUCR
		     ${OOOSDK_URE_DIR}/share/misc/types.rdb
		     ${OOOSDK_DIR}/program/offapi.rdb
		WORKING_DIRECTORY ${_OOOINCLUDE} 
	)
	add_custom_target(_oooheaders ALL
		DEPENDS ${_OOOINCLUDE}/com/sun/star/uno/Exception.hpp)

	set(OOOSDK_INCLUDE_DIRS ${CPPUHELPER_INCLUDE_DIR} ${_OOOINCLUDE})
endif(CPPUHELPER_INCLUDE_DIR AND OOOSDK_DIR AND OOOSDK_URE_DIR AND OOOSDK_LIBRARIES)

if(NOT OOoSDK_FOUND)
	if(OOoSDK_FIND_REQUIRED)
		message(FATAL_ERROR ${OOOSDK_ERROR})
	else(OOoSDK_FIND_REQUIRED)
		message(STATUS ${OOOSDK_ERROR})
	endif(OOoSDK_FIND_REQUIRED)
else(NOT OOoSDK_FOUND)
	message(STATUS "Found OpenOffice SDK.")
endif(NOT OOoSDK_FOUND)

