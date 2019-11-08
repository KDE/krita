set(_YUV_ROOT_PATHS
	${CMAKE_INSTALL_PREFIX}
)

find_path(YUV_INCLUDE_DIRS
	NAMES libyuv.h
	HINTS _YUV_ROOT_PATHS
	PATH_SUFFIXES include
)
if(YUV_INCLUDE_DIRS)
	set(HAVE_YUV_H 1)
endif()

find_library(YUV_LIBRARIES
	NAMES yuv
	HINTS _YUV_ROOT_PATHS
	PATH_SUFFIXES bin lib lib/Win32
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YUV
	DEFAULT_MSG
	YUV_INCLUDE_DIRS YUV_LIBRARIES HAVE_YUV_H
)

mark_as_advanced(YUV_INCLUDE_DIRS YUV_LIBRARIES HAVE_YUV_H)