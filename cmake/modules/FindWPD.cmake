# - Try to find the WPD graphics library
# Once done this will define
#
#  WPD_FOUND - system has WPD
#  WPD_INCLUDE_DIR - the WPD include directory
#  WPD_LIBRARIES - Link these to use WPD
#  WPD_DEFINITIONS - Compiler switches required for using WPD
#


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls

if (WPD_INCLUDE_DIR AND WPD_LIBRARIES)

   # Already in cache
   set(WPD_FOUND TRUE)

else (WPD_INCLUDE_DIR AND WPD_LIBRARIES)
if(NOT WIN32)
  INCLUDE(UsePkgConfig)
  PKGCONFIG(libwpd-0.8 _WPDIncDir _WPDLinkDir _WPDLinkFlags _WPDCflags)

  set(WPD_DEFINITIONS ${_WPDCflags} CACHE INTERNAL "The definitions for libwpd")
endif(NOT WIN32)

  FIND_PATH(WPD_INCLUDE_DIR libwpd/libwpd.h
    ${_WPDIncDir}
    /usr/include/libwpd-0.8
  )

  FIND_LIBRARY(WPD_LIBRARIES NAMES wpd-0.8
    PATHS
    ${_WPDLinkDir}
  )

  if (WPD_INCLUDE_DIR AND WPD_LIBRARIES)
     set(WPD_FOUND TRUE)
  endif (WPD_INCLUDE_DIR AND WPD_LIBRARIES)

  if (WPD_FOUND)
    if (NOT WPD_FIND_QUIETLY)
      message(STATUS "Found WPD: ${WPD_LIBRARIES}")
    endif (NOT WPD_FIND_QUIETLY)
  else (WPD_FOUND)
    if (WPD_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find WPD")
    endif (WPD_FIND_REQUIRED)
  endif (WPD_FOUND)

  MARK_AS_ADVANCED(WPD_INCLUDE_DIR WPD_LIBRARIES)

endif (WPD_INCLUDE_DIR AND WPD_LIBRARIES)
