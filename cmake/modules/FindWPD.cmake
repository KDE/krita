# - Try to find the libwpd (WordPerfect library)
# Once done this will define
#
#  WPD_FOUND - system has WPD
#  WPD_INCLUDE_DIR - the WPD include directory
#  WPD_LIBRARIES - Link these to use WPD
#  WPD_DEFINITIONS - Compiler switches required for using WPD
#


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls

#if (WPD_INCLUDE_DIR AND WPD_LIBRARIES)

   # Already in cache
#   set(WPD_FOUND TRUE)

#else (WPD_INCLUDE_DIR AND WPD_LIBRARIES)
if(NOT WIN32)
  INCLUDE(FindPkgConfig)
  pkg_check_modules(WPD libwpd-0.8)
  pkg_check_modules(WPD libwpd-0.9)
endif(NOT WIN32)

  FIND_PATH(WPD_INCLUDE_DIR libwpd/libwpd.h
    PATHS
	  ${WPD_INCLUDE_DIR}
      /usr/include/libwpd-0.8
      /usr/include/libwpd-0.9
	PATH_SUFFIXES
	  libwpd-0.8
	  libwpd-0.9
  )

  IF(NOT WPD_LIBRARIES)
    FIND_LIBRARY(WPD_LIBRARY
	  NAMES
	    wpd
		libwpd
	)

	FIND_LIBRARY(WPD_STREAM_LIBRARY
	  NAMES
	    wpd-stream
		libwpd-stream
	)
	
	set(WPD_LIBRARIES ${WPD_LIBRARY} ${WPD_STREAM_LIBRARY})
  ENDIF(NOT WPD_LIBRARIES)
  
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(WPD DEFAULT_MSG WPD_INCLUDE_DIR WPD_LIBRARIES )

  if(WPD_INCLUDE_DIR AND WPD_LIBRARIES)
    set(WPD_FOUND TRUE)
  else (WPD_INCLUDE_DIR AND WPD_LIBRARIES)
    set(WPD_FOUND FALSE)
  endif(WPD_INCLUDE_DIR AND WPD_LIBRARIES)

#endif (WPD_INCLUDE_DIR AND WPD_LIBRARIES)
