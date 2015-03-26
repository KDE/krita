# - Try to find the poppler PDF library
# Once done this will define
#
#  POPPLER_FOUND - system has poppler
#  POPPLER_INCLUDE_DIR - the poppler include directory
#  POPPLER_LIBRARY - Link this to use poppler
#

# Copyright (c) 2006-2010, Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(POPPLER_INCLUDE_DIR AND POPPLER_LIBRARY)

  # in cache already
  set(POPPLER_FOUND TRUE)

else()

set(_poppler_version_bad FALSE)

if(NOT WIN32)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)
  pkg_check_modules(_pc_poppler poppler-qt5)
  if(_pc_poppler_FOUND)
    if(NOT "${_pc_poppler_VERSION}" VERSION_GREATER 0.5.3)
      set(_poppler_version_bad TRUE)
    endif()
  endif()
endif()

if(NOT _poppler_version_bad)
  set(POPPLER_FOUND FALSE)

  find_library(POPPLER_LIBRARY poppler-qt5
               HINTS ${_pc_poppler_LIBRARY_DIRS}
  )

  find_path(POPPLER_INCLUDE_DIR poppler-qt5.h
            HINTS ${_pc_poppler_INCLUDE_DIRS}
            PATH_SUFFIXES poppler/qt5
  )
  find_path(POPPLER_INCLUDE_DIR_core qt5/poppler-qt5.h
            HINTS ${_pc_poppler_INCLUDE_DIRS}
            PATH_SUFFIXES poppler
  )

  if(POPPLER_LIBRARY AND POPPLER_INCLUDE_DIR AND POPPLER_INCLUDE_DIR_core)
    list(APPEND POPPLER_INCLUDE_DIR "${POPPLER_INCLUDE_DIR_core}")
    set(POPPLER_FOUND TRUE)
  endif()
endif()

if (POPPLER_FOUND)
  include(CheckCXXSourceCompiles)

  set(CMAKE_REQUIRED_INCLUDES ${POPPLER_INCLUDE_DIR} ${QT_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${POPPLER_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY})

check_cxx_source_compiles("
#include <poppler-qt5.h>
int main()
{
  Poppler::Document::RenderHint hint = Poppler::Document::ThinLineSolid;
  return 0;
}
" HAVE_POPPLER_0_24)

check_cxx_source_compiles("
#include <poppler-qt5.h>
int main()
{
  Poppler::Page *p = 0;
  p->annotations( QSet<Poppler::Annotation::SubType>() << Poppler::Annotation::ASound );
  return 0;
}
" HAVE_POPPLER_0_28)

  set(CMAKE_REQUIRED_INCLUDES)
  set(CMAKE_REQUIRED_LIBRARIES)
  if (HAVE_POPPLER_0_28)
    set(popplerVersionMessage "0.28")
  elseif (HAVE_POPPLER_0_24)
    set(popplerVersionMessage "0.24")
  endif ()
  if (NOT Poppler_FIND_QUIETLY)
    message(STATUS "Found Poppler-Qt5: ${POPPLER_LIBRARY}, (>= ${popplerVersionMessage})")
  endif ()
else ()
  if (Poppler_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Poppler-Qt5")
  endif ()
  message(STATUS "Could not find OPTIONAL package Poppler-Qt5")
endif ()

# ensure that they are cached
set(POPPLER_INCLUDE_DIR ${POPPLER_INCLUDE_DIR} CACHE INTERNAL "The Poppler-Qt5 include path")
set(POPPLER_LIBRARY ${POPPLER_LIBRARY} CACHE INTERNAL "The Poppler-Qt5 library")

endif()
