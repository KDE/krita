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
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    INCLUDE(UsePkgConfig)
    PKGCONFIG(libexif _LibexifIncDir _LibexifLinkDir _LibexifLinkFlags _LibexifCflags)
   endif(NOT WIN32)

  if(_LibexifLinkFlags)
    # query pkg-config asking for a libexif >= 0.6.12
    EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=0.6.12 libexif RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
    if(_return_VALUE STREQUAL "0")
      set(LIBEXIF_FOUND TRUE)
    endif(_return_VALUE STREQUAL "0")
  endif(_LibexifLinkFlags)

  if (LIBEXIF_FOUND)
    set(LIBEXIF_INCLUDE_DIR ${_LibexifIncDir} CACHE INTERNAL "The libexif include path")
    set(LIBEXIF_LIBRARY ${_LibexifLinkFlags} CACHE INTERNAL "The libexif library")
    if (NOT Exif_FIND_QUIETLY)
      message(STATUS "Found libexif: ${LIBEXIF_LIBRARY}")
    endif (NOT Exif_FIND_QUIETLY)
  else (LIBEXIF_FOUND)
    if (Exif_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find libexif")
    endif (Exif_FIND_REQUIRED)
  endif (LIBEXIF_FOUND)

  MARK_AS_ADVANCED(LIBEXIF_INCLUDE_DIR LIBEXIF_LIBRARY)

endif(LIBEXIF_INCLUDE_DIR AND LIBEXIF_LIBRARY)
