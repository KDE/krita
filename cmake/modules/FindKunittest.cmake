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

if (KUNITTEST_INCLUDE_DIR AND KUNITTEST_LIBRARIES)
   set(KUNITTEST_FOUND TRUE)
endif (KUNITTEST_INCLUDE_DIR AND KUNITTEST_LIBRARIES)


if (KUNITTEST_FOUND)
   if (NOT Kunittest_FIND_QUIETLY)
      message(STATUS "Found kunittest: ${KUNITTEST_LIBRARIES}")
   endif (NOT Kunittest_FIND_QUIETLY)
else(KUNITTEST_FOUND)   
   if(Kunittest_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find kunitest")
   endif(Kunittest_FIND_REQUIRED)
endif (KUNITTEST_FOUND)

MARK_AS_ADVANCED(KUNITTEST_INCLUDE_DIR KUNITTEST_LIBRARIES)

