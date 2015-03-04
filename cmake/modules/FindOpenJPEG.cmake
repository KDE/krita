# -*- cmake -*-

# - Find OpenJPEG
# Find the OpenJPEG includes and library
# This module defines
#  OPENJPEG_INCLUDE_DIR, where to find openjpeg.h, etc.
#  OPENJPEG_LIBRARIES, the libraries needed to use OpenJPEG.
#  OPENJPEG_FOUND, If false, do not try to use OpenJPEG.
# also defined, but not for general use are
#  OPENJPEG_LIBRARY, where to find the OpenJPEG library.

find_path(OPENJPEG_INCLUDE_DIR openjpeg.h
PATHS
    /usr/local/include/openjpeg
    /usr/local/include
    /usr/include/openjpeg
    /usr/include/openjpeg-1.5
    /usr/include
PATH_SUFFIXES
    openjpeg-1.5
    openjpeg
)

set(OPENJPEG_NAMES ${OPENJPEG_NAMES} openjpeg)
find_library(OPENJPEG_LIBRARY
  NAMES ${OPENJPEG_NAMES}
  PATHS
      /usr/lib /usr/local/lib
)

if (OPENJPEG_LIBRARY AND OPENJPEG_INCLUDE_DIR)
    set(OPENJPEG_LIBRARIES ${OPENJPEG_LIBRARY})
    set(OPENJPEG_FOUND "YES")
else ()
  set(OPENJPEG_FOUND "NO")
endif ()


if (OPENJPEG_FOUND)
   if (NOT OPENJPEG_FIND_QUIETLY)
      message(STATUS "Found OpenJPEG: ${OPENJPEG_LIBRARIES}")
   endif ()
else ()
   if (OPENJPEG_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OpenJPEG library")
   endif ()
endif ()

# Deprecated declarations.
set (NATIVE_OPENJPEG_INCLUDE_PATH ${OPENJPEG_INCLUDE_DIR} )
get_filename_component (NATIVE_OPENJPEG_LIB_PATH ${OPENJPEG_LIBRARY} PATH)

mark_as_advanced(
  OPENJPEG_LIBRARY
  OPENJPEG_INCLUDE_DIR
  )
