# - Try to find the libexif graphics library
# Once done this will define
#
#  LIBEXIF_FOUND - system has LIBEXIF
#  LIBEXIF_INCLUDE_DIR - the LIBEXIF include directory
#  LIBEXIF_LIBRARY - Link this to use LIBEXIF
#


if(LIBEXIF_INCLUDE_DIR AND LIBEXIF_LIBRARY)

  # Already in cache
  set(LIBEXIF_FOUND TRUE)

else(LIBEXIF_INCLUDE_DIR AND LIBEXIF_LIBRARY)
  if(NOT WIN32)
    INCLUDE(FindPkgConfig)
    pkg_check_modules(LIBEXIF libexif>=0.6.12)
   endif(NOT WIN32)

  if (LIBEXIF_FOUND)
    if (NOT Exif_FIND_QUIETLY)
      message(STATUS "Found libexif: ${LIBEXIF_LIBRARY}")
    endif ()
  else ()
    if (Exif_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find libexif")
    endif ()
  endif ()

  MARK_AS_ADVANCED(LIBEXIF_INCLUDE_DIR LIBEXIF_LIBRARY)

endif(LIBEXIF_INCLUDE_DIR AND LIBEXIF_LIBRARY)
