# - Find GSL
# Find the GNU Scientific Library (GSL) includes and library
#
# This module defines
#  GSL_FOUND
#  GSL_LIBRARIES
#  GSL_CBLAS_LIBRARIES
#  GSL_INCLUDE_DIR
#  GSL_VERSION
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(UsePkgConfig)

if (GSL_INCLUDE_DIR AND GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES AND GSL_VERSION)

	# Already in cache
	SET(GSL_FOUND TRUE)

else ()

	FIND_LIBRARY (GSL_LIBRARIES gsl)

	FIND_LIBRARY (GSL_CBLAS_LIBRARIES gslcblas)

	FIND_PATH (GSL_INCLUDE_DIR gsl_multimin.h
		/usr/include
		/usr/local/include
        ${CMAKE_INCLUDE_PATH}
		PATH_SUFFIXES gsl
	)

	if (NOT WIN32)
		FIND_PROGRAM (GSL_CONFIG gsl-config)

		IF (GSL_CONFIG)
			EXEC_PROGRAM (${GSL_CONFIG} ARGS "--version" OUTPUT_VARIABLE gsl_version)
		#	EXEC_PROGRAM (${GSL_CONFIG} ARGS "--cflags" OUTPUT_VARIABLE gsl_include_dir)
		#	EXEC_PROGRAM (${GSL_CONFIG} ARGS "--libs" OUTPUT_VARIABLE gsl_libraries)

		#	STRING (REGEX REPLACE "-I([^ ]*)" "\\1" GSL_INCLUDE_DIR "${gsl_include_dir}")
		#	STRING (REGEX REPLACE "-L([^ ]*)" "\\1" GSL_LIBRARIES "${gsl_libraries}")
			SET (GSL_VERSION ${gsl_version} CACHE STRING "GNU Scientific Library Version")
			# TODO check version! 1.6 suffices?
		ENDIF (GSL_CONFIG)
	else ()
		file(STRINGS ${GSL_INCLUDE_DIR}/gsl_version.h gsl_version REGEX "^#define GSL_VERSION \"(.+)\"$")
		string(REGEX REPLACE "^#define GSL_VERSION \"(.+)\"$" "\\1" gsl_version "${gsl_version}")
		SET (GSL_VERSION ${gsl_version} CACHE STRING "GNU Scientific Library Version")
	endif ()

	IF (NOT GSL_INCLUDE_DIR OR NOT GSL_VERSION)
		# Try pkg-config instead, but only for the
		# include directory, since we really need
		# the *LIBRARIES to point to the actual .so's.
		PKGCONFIG(gsl GSL_INCLUDE_DIR dummy dummy dummy)
	ENDIF (NOT GSL_INCLUDE_DIR OR NOT GSL_VERSION)
	#
	# everything necessary found?
	#
	IF (GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES AND GSL_INCLUDE_DIR)
		SET (GSL_FOUND TRUE)
	ELSE (GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES AND GSL_INCLUDE_DIR)
		SET (GSL_FOUND FALSE)
	ENDIF (GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES AND GSL_INCLUDE_DIR)

endif ()


#
# output status
#
if (GSL_FOUND)
     if (NOT GSL_FIND_QUIETLY)
        MESSAGE (STATUS "Found GNU Scientific Library: ${GSL_VERSION} (${GSL_INCLUDE_DIR} ${GSL_LIBRARIES};${GSL_CBLAS_LIBRARIES})")
     endif()
else ()
     if (GSL_FIND_REQUIRED)
	MESSAGE (FATAL_ERROR "Required package gsl NOT found")
     endif ()
endif ()

