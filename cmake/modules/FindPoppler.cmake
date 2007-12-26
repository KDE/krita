# - Try to find the poppler PDF library
# Once done this will define
#
#  POPPLER_FOUND - system has poppler
#  POPPLER_INCLUDE_DIR - the poppler include directory
#  POPPLER_LIBRARY - Link this to use poppler
#

# Copyright (c) 2006-2007, Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(POPPLER_INCLUDE_DIR AND POPPLER_LIBRARY)

  # in cache already
  set(POPPLER_FOUND TRUE)

else(POPPLER_INCLUDE_DIR AND POPPLER_LIBRARY)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
INCLUDE(UsePkgConfig)

PKGCONFIG(poppler-qt4 _PopplerIncDir _PopplerLinkDir _PopplerLinkFlags _PopplerCflags)

if(_PopplerLinkFlags)
  # find again pkg-config, to query it about poppler version
  FIND_PROGRAM(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS /usr/bin/ /usr/local/bin )

  # query pkg-config asking for a poppler-qt4 >= 0.5.4
  EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=0.5.4 poppler-qt4 RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
  if(_return_VALUE STREQUAL "0")
    set(POPPLER_FOUND TRUE)
  endif(_return_VALUE STREQUAL "0")
else(_PopplerLinkFlags)
  # try to find poppler without pkgconfig
  find_library( LIBPOPPLER poppler )
  find_library( LIBPOPPLER_QT4 poppler-qt4 )
  find_path( INCLUDEPOPPLER_QT4 poppler/qt4/poppler-qt4.h )
  find_path( INCLUDEPOPPLER poppler-qt4.h PATHS ${INCLUDEPOPPLER_QT4}/poppler/qt4 )
  if( LIBPOPPLER_QT4 AND LIBPOPPLER AND INCLUDEPOPPLER )
    set( POPPLER_FOUND TRUE )
    set(_PopplerLinkFlags ${LIBPOPPLER} ${LIBPOPPLER_QT4})
    set(POPPLER_INCLUDE_DIR ${INCLUDEPOPPLER})
  endif( LIBPOPPLER_QT4 AND LIBPOPPLER AND INCLUDEPOPPLER )
endif(_PopplerLinkFlags)

if (POPPLER_FOUND)
  set(POPPLER_LIBRARY ${_PopplerLinkFlags})

  # the cflags for poppler-qt4 can contain more than one include path
  separate_arguments(_PopplerCflags)
  foreach(_includedir ${_PopplerCflags})
    string(REGEX REPLACE "-I(.+)" "\\1" _includedir "${_includedir}")
    set(POPPLER_INCLUDE_DIR ${POPPLER_INCLUDE_DIR} ${_includedir})
  endforeach(_includedir)

  # check whether we're using poppler 0.6
  set(CMAKE_REQUIRED_INCLUDES ${POPPLER_INCLUDE_DIR} ${QT_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${POPPLER_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY})
check_cxx_source_compiles("
#include <poppler-qt4.h>

int main()
{
  Poppler::SoundObject * so = 0;
  (void)so;

  return 0;
}
" HAVE_POPPLER_0_6 )
  set(CMAKE_REQUIRED_INCLUDES)
  set(CMAKE_REQUIRED_LIBRARIES)
  if (HAVE_POPPLER_0_6)
    set(poppler06Message "yes")
  else (HAVE_POPPLER_0_6)
    set(poppler06Message "no")
  endif (HAVE_POPPLER_0_6)

  if (NOT Poppler_FIND_QUIETLY)
    message(STATUS "Found Poppler-Qt4: ${POPPLER_LIBRARY}, 0.6.x? ${poppler06Message}")
  endif (NOT Poppler_FIND_QUIETLY)
else (POPPLER_FOUND)
  if (Poppler_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Poppler-Qt4")
  endif (Poppler_FIND_REQUIRED)
  message(STATUS "Could not find OPTIONAL package Poppler-Qt4")
endif (POPPLER_FOUND)

# ensure that they are cached
set(POPPLER_INCLUDE_DIR ${POPPLER_INCLUDE_DIR} CACHE INTERNAL "The Poppler-Qt4 include path")
set(POPPLER_LIBRARY ${POPPLER_LIBRARY} CACHE INTERNAL "The Poppler-Qt4 library")
set(HAVE_POPPLER_0_6 ${HAVE_POPPLER_0_6} CACHE INTERNAL "Whether the version of Poppler-Qt4 is 0.6")

endif(POPPLER_INCLUDE_DIR AND POPPLER_LIBRARY)
