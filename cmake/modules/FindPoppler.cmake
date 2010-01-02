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

INCLUDE(FindPkgConfig)
pkg_check_modules(POPPLER poppler-qt4>=0.5.4)

if(NOT POPPLER_FOUND)
  # try to find poppler without pkgconfig
  find_library( LIBPOPPLER poppler )
  find_library( LIBPOPPLER_QT4 poppler-qt4 )
  find_path( INCLUDEPOPPLER_QT4 poppler/qt4/poppler-qt4.h )
  find_path( INCLUDEPOPPLER poppler-qt4.h PATHS ${INCLUDEPOPPLER_QT4}/poppler/qt4 )
  if( LIBPOPPLER_QT4 AND LIBPOPPLER AND INCLUDEPOPPLER )
    set( POPPLER_FOUND TRUE )
    set(POPPLER_LIBRARY ${LIBPOPPLER} ${LIBPOPPLER_QT4})
    set(POPPLER_INCLUDE_DIR ${INCLUDEPOPPLER})
  endif()
endif()

if (POPPLER_FOUND)
  # the cflags for poppler-qt4 can contain more than one include path
  separate_arguments(POPPLER_CFLAGS)
  foreach(_includedir ${POPPLER_CFLAGS})
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
