# - Try to find the kunittest directory library
# Once done this will define
#
#  KUNITTEST_FOUND - system has kunittest
#  KUNITTEST_INCLUDE_DIR - the kunittest include directory
#  KUNITTEST_LIBRARIES - The libraries needed to use kunittest

FIND_PATH(KUNITTEST_INCLUDE_DIR kunittest/tester.h
   ${KDE4_INCLUDE_DIR}
)

FIND_LIBRARY(KUNITTEST_LIBRARIES NAMES kunittest
   PATHS
   ${KDE4_LIB_DIR}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Kunittest DEFAULT_MSG KUNITTEST_INCLUDE_DIR KUNITTEST_LIBRARIES )


MARK_AS_ADVANCED(KUNITTEST_INCLUDE_DIR KUNITTEST_LIBRARIES)

