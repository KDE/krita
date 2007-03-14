# - Find Grqphic Magick
# This module finds if GraphicsMagick library is installed.
# This code sets the following variables:
#
#  GRAPHICSMAGICK_COMPILE_FLAGS, the compile flags
#  GRAPHICSMAGICK_LIBRARIES, The libraries needed to use ImageMagick.
#  GRAPHICSMAGICK_VERSION, The version in the form major.minor.patch
#  GRAPHICSMAGICK_MAJOR_VERSION, The major version number
#  GRAPHICSMAGICK_MINOR_VERSION, The minor version number
#  GRAPHICSMAGICK_PATCH_VERSION, The patch version number
#  GRAPHICSMAGICK_FOUND, If false, do not try to use ImageMagick.

SET(HAVE_GMAGICK 0)
set(GRAPHICSMAGICK_FOUND FALSE)

   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   INCLUDE(UsePkgConfig)
   PKGCONFIG(GraphicsMagick _libMagickIncDir _libMagickLinkDir _libMagickLinkFlags _libMagickCflags)

   set(GRAPHICSMAGICK_COMPILE_FLAGS ${_libMagickCflags})
   set(GRAPHICSMAGICK_LIBRARIES ${_libMagickLinkFlags})

   find_program(GMAGICK_CONFIG_EXECUTABLE
        NAMES GraphicsMagick-config
        PATHS
        /usr/bin
        /usr/local/bin
        /opt/local/bin
    )


   exec_program(${GMAGICK_CONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE GRAPHICSMAGICK_VERSION)

   string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" GRAPHICSMAGICK_MAJOR_VERSION "${GRAPHICSMAGICK_VERSION}")
   string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" GRAPHICSMAGICK_MINOR_VERSION "${GRAPHICSMAGICK_VERSION}")
   string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" GRAPHICSMAGICK_PATCH_VERSION "${GRAPHICSMAGICK_VERSION}")

   if(GRAPHICSMAGICK_COMPILE_FLAGS AND GRAPHICSMAGICK_LIBRARIES AND GRAPHICSMAGICK_VERSION)
      set(GRAPHICSMAGICK_FOUND TRUE)
   endif(GRAPHICSMAGICK_COMPILE_FLAGS AND GRAPHICSMAGICK_LIBRARIES AND GRAPHICSMAGICK_VERSION)

if(GRAPHICSMAGICK_FOUND)
   set(HAVE_GMAGICK 1)
   if(NOT GraphicsMagick_FIND_QUIETLY)
      message(STATUS "Found Graphics Magick version ${GRAPHICSMAGICK_VERSION}: ${GRAPHICSMAGICK_LIBRARIES} ${GRAPHICSMAGICK_COMPILE_FLAGS}")
   endif(NOT GraphicsMagick_FIND_QUIETLY)
else(GRAPHICSMAGICK_FOUND)
   if(NOT GraphicsMagick_FIND_QUIETLY)
      if(GraphicsMagick_FIND_REQUIRED)
         message(FATAL_ERROR "Required package Graphics Magick NOT found")
      else(GraphicsMagick_FIND_REQUIRED)
         message(STATUS "Graphics Magick NOT found")
      endif(GraphicsMagick_FIND_REQUIRED)
   endif(NOT GraphicsMagick_FIND_QUIETLY)
endif(GRAPHICSMAGICK_FOUND)

mark_as_advanced(
  GRAPHICSMAGICK_COMPILE_FLAGS
  GRAPHICSMAGICK_LIBRARIES
  GRAPHICSMAGICK_VERSION
  GRAPHICSMAGICK_MAJOR_VERSION
  GRAPHICSMAGICK_MINOR_VERSION
  GRAPHICSMAGICK_PATCH_VERSION
)

