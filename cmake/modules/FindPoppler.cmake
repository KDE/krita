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

else(POPPLER_INCLUDE_DIR AND POPPLER_LIBRARY)

if(NOT WIN32)
# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)
  pkg_check_modules(_pc_poppler poppler-qt4)
  if(_pc_poppler_FOUND AND _pc_poppler_VERSION VERSION_GREATER 0.5.3)
    set(POPPLER_FOUND TRUE)
  endif(_pc_poppler_FOUND AND _pc_poppler_VERSION VERSION_GREATER 0.5.3)
else(NOT WIN32)
  # assume so, for now
  set(POPPLER_FOUND TRUE)
endif(NOT WIN32)

if(POPPLER_FOUND)
  # set it back as false
  set(POPPLER_FOUND FALSE)

  find_library(POPPLER_LIBRARY poppler-qt4
               HINTS ${_pc_poppler_LIBRARY_DIRS}
  )

  find_path(POPPLER_INCLUDE_DIR poppler-qt4.h
            HINTS ${_pc_poppler_INCLUDE_DIRS}
            PATH_SUFFIXES poppler/qt4
  )
  find_path(POPPLER_INCLUDE_DIR_core qt4/poppler-qt4.h
            HINTS ${_pc_poppler_INCLUDE_DIRS}
            PATH_SUFFIXES poppler
  )

  if(POPPLER_LIBRARY AND POPPLER_INCLUDE_DIR AND POPPLER_INCLUDE_DIR_core)
    list(APPEND POPPLER_INCLUDE_DIR "${POPPLER_INCLUDE_DIR_core}")
    set(POPPLER_FOUND TRUE)
  endif(POPPLER_LIBRARY AND POPPLER_INCLUDE_DIR AND POPPLER_INCLUDE_DIR_core)
endif(POPPLER_FOUND)

if (POPPLER_FOUND)
  INCLUDE(CheckCXXSourceCompiles)

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
check_cxx_source_compiles("
#include <poppler-qt4.h>
#include <poppler-form.h>
int main()
{
  Poppler::FormFieldButton * button = 0;
  button->buttonType();
  return 0;
}
" HAVE_POPPLER_0_8)
check_cxx_source_compiles("
#include <poppler-qt4.h>
int main()
{
  Poppler::Document::RenderHint hint = Poppler::Document::TextHinting;
  return 0;
}
" HAVE_POPPLER_0_12_1)
  set(CMAKE_REQUIRED_INCLUDES)
  set(CMAKE_REQUIRED_LIBRARIES)
  if (HAVE_POPPLER_0_12_1)
    set(popplerVersionMessage "0.12.1")
  elseif (HAVE_POPPLER_0_8)
    set(popplerVersionMessage "0.8")
  elseif (HAVE_POPPLER_0_6)
    set(popplerVersionMessage "0.6")
  else (HAVE_POPPLER_0_12_1)
    set(popplerVersionMessage "0.5.4")
  endif (HAVE_POPPLER_0_12_1)
  if (NOT Poppler_FIND_QUIETLY)
    message(STATUS "Found Poppler-Qt4: ${POPPLER_LIBRARY}, (>= ${popplerVersionMessage})")
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
set(HAVE_POPPLER_0_6 ${HAVE_POPPLER_0_6} CACHE INTERNAL "Whether the version of Poppler-Qt4 is >= 0.6")
set(HAVE_POPPLER_0_8 ${HAVE_POPPLER_0_8} CACHE INTERNAL "Whether the version of Poppler-Qt4 is >= 0.8")
set(HAVE_POPPLER_0_12_1 ${HAVE_POPPLER_0_12_1} CACHE INTERNAL "Whether the version of Poppler-Qt4 is >= 0.12.1")

endif(POPPLER_INCLUDE_DIR AND POPPLER_LIBRARY)
