# - Find Image Magick
# This module finds if ImageMagick tools are installed and determines
# where the executables are. This code sets the following variables:
#
#  IMAGEMAGICK_CONVERT_EXECUTABLE   =
#     the full path to the 'convert' utility
#  IMAGEMAGICK_MOGRIFY_EXECUTABLE   =
#     the full path to the 'mogrify' utility
#  IMAGEMAGICK_IMPORT_EXECUTABLE    =
#     the full path to the 'import'  utility
#  IMAGEMAGICK_MONTAGE_EXECUTABLE   =
#     the full path to the 'montage' utility
#  IMAGEMAGICK_COMPOSITE_EXECUTABLE =
#     the full path to the 'composite' utility
#
#  IMAGEMAGICK_INCLUDE_DIR, Where to find the ImageMagick headers.
#  IMAGEMAGICK_LIBRARIES, The libraries needed to use ImageMagick.
#  IMAGEMAGICK_VERSION, The version in the form major.minor.patch
#  IMAGEMAGICK_MAJOR_VERSION, The major version number
#  IMAGEMAGICK_MINOR_VERSION, The minor version number
#  IMAGEMAGICK_PATCH_VERSION, The patch version number
#  IMAGEMAGICK_FOUND, If false, do not try to use ImageMagick.

if(IMAGEMAGICK_INCLUDE_DIR AND IMAGEMAGICK_LIBRARIES AND IMAGEMAGICK_VERSION)

  # in cache already
  set(HAVE_MAGICK 1)
  set(IMAGEMAGICK_FOUND TRUE)

  # and the executables are in the cache too, if they were found.

else(IMAGEMAGICK_INCLUDE_DIR AND IMAGEMAGICK_LIBRARIES AND IMAGEMAGICK_VERSION)

  SET(HAVE_MAGICK 0)

  if (WIN32)

    # Try to find the ImageMagick binary path.

    FIND_PATH(IMAGEMAGICK_BINARY_PATH mogrify.exe
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]
      DOC "Path to the ImageMagick binary directory where all executable should be found."
    )

    # Be extra-careful here: we do NOT want CMake to look in the system's PATH
    # env var to search for convert.exe, otherwise it is going to pick
    # Window's own convert.exe, and you may say good-bye to your disk.

    FIND_PROGRAM(IMAGEMAGICK_CONVERT_EXECUTABLE
      NAMES convert
      PATHS ${IMAGEMAGICK_BINARY_PATH}
      NO_SYSTEM_PATH
      DOC "Path to ImageMagick's convert executable. WARNING: note that this tool, named convert.exe, conflicts with Microsoft Window's own convert.exe, which is used to convert FAT partitions to NTFS format ! Therefore, be extra-careful and make sure the right convert.exe has been picked."
    )

  ELSE (WIN32)

    SET (IMAGEMAGICK_BINARY_PATH "")

    FIND_PROGRAM(IMAGEMAGICK_CONVERT_EXECUTABLE
      NAMES convert
      PATHS ${IMAGEMAGICK_BINARY_PATH}
      DOC "Path to ImageMagick's convert executable."
    )

  ENDIF (WIN32)

  # Find mogrify, import, montage, composite

  FIND_PROGRAM(IMAGEMAGICK_MOGRIFY_EXECUTABLE
    NAMES mogrify
    PATHS ${IMAGEMAGICK_BINARY_PATH}
    DOC "Path to ImageMagick's mogrify executable."
  )

  FIND_PROGRAM(IMAGEMAGICK_IMPORT_EXECUTABLE
    NAMES import
    PATHS ${IMAGEMAGICK_BINARY_PATH}
    DOC "Path to ImageMagick's import executable."
  )

  FIND_PROGRAM(IMAGEMAGICK_MONTAGE_EXECUTABLE
    NAMES montage
    PATHS ${IMAGEMAGICK_BINARY_PATH}
    DOC "Path to ImageMagick's montage executable."
  )

  FIND_PROGRAM(IMAGEMAGICK_COMPOSITE_EXECUTABLE
    NAMES composite
    PATHS ${IMAGEMAGICK_BINARY_PATH}
    DOC "Path to ImageMagick's composite executable."
  )

  find_program(MAGICK_CONFIG_EXECUTABLE
     NAMES Magick-config
     PATHS
     ${IMAGEMAGICK_BINARY_PATH}
     /opt/local/bin
  )

  set(IMAGEMAGICK_FOUND FALSE)

  if(MAGICK_CONFIG_EXECUTABLE)
     if(NOT WIN32)
       INCLUDE(FindPkgConfig)
       pkg_check_modules(IMAGEMAGICK ImageMagick)
     else()
	exec_program(${MAGICK_CONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE IMAGEMAGICK_VERSION)
     endif(NOT WIN32)

     find_path(IMAGEMAGICK_INCLUDE_DIR magick/api.h
        /opt/local/include
     )

     string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" IMAGEMAGICK_MAJOR_VERSION "${IMAGEMAGICK_VERSION}")
     string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" IMAGEMAGICK_MINOR_VERSION "${IMAGEMAGICK_VERSION}")
     string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" IMAGEMAGICK_PATCH_VERSION "${IMAGEMAGICK_VERSION}")

     set(IMAGEMAGICK_VERSION ${IMAGEMAGICK_VERSION} CACHE INTERNAL "The image magick version")

     if(IMAGEMAGICK_INCLUDE_DIR AND IMAGEMAGICK_LIBRARIES AND IMAGEMAGICK_VERSION)
        set(IMAGEMAGICK_FOUND TRUE)
     endif(IMAGEMAGICK_INCLUDE_DIR AND IMAGEMAGICK_LIBRARIES AND IMAGEMAGICK_VERSION)
  endif(MAGICK_CONFIG_EXECUTABLE)

  if(IMAGEMAGICK_FOUND)
     set(HAVE_MAGICK 1)
     if(NOT ImageMagick_FIND_QUIETLY)
        message(STATUS "Found Image Magick version ${IMAGEMAGICK_VERSION}: ${IMAGEMAGICK_LIBRARIES}")
     endif(NOT ImageMagick_FIND_QUIETLY)
  else(IMAGEMAGICK_FOUND)
     if(NOT ImageMagick_FIND_QUIETLY)
        if(ImageMagick_FIND_REQUIRED)
           message(FATAL_ERROR "Required package Image Magick NOT found")
        else(ImageMagick_FIND_REQUIRED)
           message(STATUS "Image Magick NOT found")
        endif(ImageMagick_FIND_REQUIRED)
     endif(NOT ImageMagick_FIND_QUIETLY)
  endif(IMAGEMAGICK_FOUND)

  mark_as_advanced(
    IMAGEMAGICK_BINARY_PATH
    IMAGEMAGICK_CONVERT_EXECUTABLE
    IMAGEMAGICK_MOGRIFY_EXECUTABLE
    IMAGEMAGICK_IMPORT_EXECUTABLE
    IMAGEMAGICK_MONTAGE_EXECUTABLE
    IMAGEMAGICK_COMPOSITE_EXECUTABLE
    IMAGEMAGICK_INCLUDE_DIR
    IMAGEMAGICK_LIBRARIES
    IMAGEMAGICK_VERSION
    IMAGEMAGICK_MAJOR_VERSION
    IMAGEMAGICK_MINOR_VERSION
    IMAGEMAGICK_PATCH_VERSION
  )

endif(IMAGEMAGICK_INCLUDE_DIR AND IMAGEMAGICK_LIBRARIES AND IMAGEMAGICK_VERSION)
