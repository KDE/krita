# - Find FFmpeg library
# Find the native FFmpeg includes and library.
# 
# This module defines:
# FFMPEG_FOUND
# FFMPEG_INCLUDE_DIR - The FFmpeg include directory.
# FFMPEG_LIBRARIES - The FFmpeg libraries.
# FFMPEG_DEFINITIONS - The FFmpeg compile flags.
# 
# For each component:
# ${COMPONENT}_FOUND
# ${COMPONENT}_INCLUDE_DIR
# ${COMPONENT}_LIBRARIES
# ${COMPONENT}_DEFINITIONS
# ${COMPONENT}_VERSION
#
# If any component is specified, default components are avcodec avformat avutil.
#

### Macro: check_ffmpeg_version
#	
# Check FFmpeg version.
# @todo: clean how to get current FFmpeg version.
#
macro(check_ffmpeg_version)
	exec_program("ffmpeg -version 2>&1 | grep 'ffmpeg version' | sed -e 's/ffmpeg\ version\ //'"
		OUTPUT_VARIABLE FFmpeg_CURRENT_VERSION)
	if(${FFmpeg_CURRENT_VERSION} LESS ${FFmpeg_FIND_VERSION})
		message(SEND_ERROR "Your FFmpeg version is too old (${FFmpeg_CURRENT_VERSION} < ${FFmpeg_FIND_VERSION}).")
	endif()
endmacro()


### Macro: find_component
#
# Check the given component by invoking pkgconfig and
# then looking up the libraries and include directories.
#
macro(find_component COMPONENT PKGCONFIG LIBRARY HEADER)
	if(NOT WIN32)
		# use pkg-config to get the directories
		find_package(PkgConfig)
		if(PKG_CONFIG_FOUND)
			pkg_check_modules(PC_${COMPONENT} ${PKGCONFIG})
		endif()
		# find include
		find_path(${COMPONENT}_INCLUDE_DIR
			${HEADER}
			HINTS ${PC_${COMPONENT}_INCLUDEDIR} ${PC_${COMPONENT}_INCLUDE_DIR}
		)
		# find lib
		find_library(${COMPONENT}_LIBRARIES
			NAMES ${LIBRARY}
			HINTS ${PC_${COMPONENT}_LIBDIR} ${PC_${COMPONENT}_LIBRARY_DIRS}
		)
		# set definition and version
		set(${COMPONENT}_DEFINITIONS ${PC_${COMPONENT}_CFLAGS_OTHER} CACHE STRING "The ${COMPONENT} CFLAGS.")
		set(${COMPONENT}_VERSION ${PC_${COMPONENT}_VERSION} CACHE STRING "The ${COMPONENT} version number.")
	else()
		# find include
		find_path(${COMPONENT}_INCLUDE_DIR ${HEADER})
		# find lib
		file(GLOB_RECURSE ${COMPONENT}_LIBRARIES "${CMAKE_PREFIX_PATH}/*${COMPONENT}.lib") 
		# @todo: define definition and version
	endif()

	# Marks the given component as found if both *_LIBRARIES AND *_INCLUDE_DIR is present.
	if(${COMPONENT}_LIBRARIES AND ${COMPONENT}_INCLUDE_DIR)
		set(${COMPONENT}_FOUND TRUE)
	else()
		set(${COMPONENT}_FOUND FALSE)
	endif()

	mark_as_advanced(
		${COMPONENT}_FOUND
		${COMPONENT}_INCLUDE_DIR
		${COMPONENT}_LIBRARIES
		${COMPONENT}_DEFINITIONS
		${COMPONENT}_VERSION
	)
endmacro()

### Macro: manage_components
# Define CMake variables for each component defined in FFmpeg_FIND_COMPONENTS
# Send error if a required component is missing (warning if not required)
# Set if FFMPEG is found
#
macro(manage_components)
	# Check components and add their stuff to the FFMPEG_* vars.
	foreach(COMPONENT ${FFmpeg_FIND_COMPONENTS})
		# Get component name is lower cases.
		string(TOLOWER ${COMPONENT} LOWERCOMPONENT)
		find_component(${COMPONENT} lib${LOWERCOMPONENT} ${LOWERCOMPONENT} lib${LOWERCOMPONENT}/${LOWERCOMPONENT}.h)
		# If the component is found.
		if(${COMPONENT}_FOUND)
			message(STATUS "Component ${COMPONENT} present.")
			# Skip components which are in a different location
			# This prevents us to depend on libav system libraries if we build with ffmpeg (and the reverse).
			if(NOT FFMPEG_INCLUDE_DIR OR FFMPEG_INCLUDE_DIR STREQUAL ${${COMPONENT}_INCLUDE_DIR})
				set(FFMPEG_INCLUDE_DIR ${${COMPONENT}_INCLUDE_DIR})
				set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${${COMPONENT}_LIBRARIES})
				set(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${${COMPONENT}_DEFINITIONS})
			else()
				message(STATUS "Skip ${COMPONENT} component because it was found elsewhere ('${${COMPONENT}_INCLUDE_DIR}' instead of '${FFMPEG_INCLUDE_DIR}').")
				set(${COMPONENT}_FOUND FALSE)
			endif()
		else()
			if(FFmpeg_FIND_REQUIRED)
				message(SEND_ERROR "Error: required component ${COMPONENT} missing.")
			else()
				message(STATUS "Warning: component ${COMPONENT} missing.")
			endif()
		endif()
	endforeach()

	# Build the include path with duplicates removed.
	if(FFMPEG_INCLUDE_DIR)
		list(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIR)
		set(FFMPEG_FOUND TRUE)
	endif()
endmacro()

# Check FFmpeg version
if(DEFINED FFmpeg_FIND_VERSION)
	check_ffmpeg_version()
endif()

# Get basic components if no one is indicated
if(NOT FFmpeg_FIND_COMPONENTS)
	set(FFmpeg_FIND_COMPONENTS avcodec avformat avutil)
endif()

# Check each component
manage_components()
