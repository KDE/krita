# - Find GSL
# Find the GNU Scientific Library (GSL) includes and library
#
# This module defines
#  GSL_FOUND
#  GSL_LIBRARIES
#  GSL_INCLUDE_DIR
#  GSL_VERSION
#

FIND_LIBRARY (GSL_LIBRARIES gsl
	PATHS
	/usr/lib
	/usr/local/lib
)

FIND_LIBRARY (GSL_CBLAS_LIBRARIES gslcblas
	PATHS
	/usr/lib
	/usr/local/lib
)

FIND_PATH (GSL_INCLUDE_DIR gsl_multimin.h
	/usr/include/gsl
	/usr/local/include/gsl
)

FIND_PROGRAM (GSL_CONFIG gsl-config
	/usr/bin
	/usr/local/bin
)

IF (GSL_CONFIG)
	EXEC_PROGRAM (${GSL_CONFIG} ARGS "--version" OUTPUT_VARIABLE GSL_VERSION)
#	EXEC_PROGRAM (${GSL_CONFIG} ARGS "--cflags" OUTPUT_VARIABLE gsl_include_dir)
#	EXEC_PROGRAM (${GSL_CONFIG} ARGS "--libs" OUTPUT_VARIABLE gsl_libraries)

#	STRING (REGEX REPLACE "-I([^ ]*)" "\\1" GSL_INCLUDE_DIR "${gsl_include_dir}")
#	STRING (REGEX REPLACE "-L([^ ]*)" "\\1" GSL_LIBRARIES "${gsl_libraries}")
ENDIF (GSL_CONFIG)


#
# everything necessary found?
#
IF (GSL_LIBRARIES AND GSL_INCLUDE_DIR)
	MESSAGE (STATUS "Found GNU Scientific Library ${GSL_VERSION}: ${GSL_INCLUDE_DIR}, ${GSL_LIBRARIES}")
	SET (GSL_FOUND TRUE)
ELSE (GSL_LIBRARIES AND GSL_INCLUDE_DIR)
	MESSAGE (STATUS "GNU Scientific Library not found.")
	SET (GSL_FOUND FALSE)
ENDIF (GSL_LIBRARIES AND GSL_INCLUDE_DIR)
