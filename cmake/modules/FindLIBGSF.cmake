# - Try to find libGSF
#
# Once done this will define
#
#  LIBGSF_FOUND - System has LibGSF
#  LIBGSF_INCLUDE_DIR - The LibGSF include directory
#  LIBGSF_LIBRARIES - The libraries needed to use LibGSF
#  LIBGSF_DEFINITIONS - Compiler switches required for using LibGSF
#  LIBGSF_GSF_EXECUTABLE - The archive utility
#  LIBGSF_GSFOFFICETHUMBNAILER_EXECUTABLE - The office files thumbnailer for the GNOME desktop
#  LIBGSF_GSFVBADUMP_EXECUTABLE - The utility to extract Visual Basic for Applications macros

# Copyright (c) 2009, Pau Garcia i Quiles <pgquiles@elpauer.org>
# Based off FindLibXml2.cmake from CMake 2.6.4 by Alexander Neundorf <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (LIBGSF_INCLUDE_DIR AND LIBGSF_LIBRARIES)
   # in cache already
   SET(LIBGSF_FIND_QUIETLY TRUE)
ENDIF (LIBGSF_INCLUDE_DIR AND LIBGSF_LIBRARIES)

IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   FIND_PACKAGE(PkgConfig)
   PKG_CHECK_MODULES(PC_LIBGSF libgsf-1)
   SET(LIBGSF_DEFINITIONS ${PC_LIBGSF_CFLAGS_OTHER})
ENDIF (NOT WIN32)

FIND_PATH(LIBGSF_INCLUDE_DIR gsf/gsf.h
   HINTS
   ${PC_LIBGSF_INCLUDEDIR}
   ${PC_LIBGSF_INCLUDE_DIRS}
   PATH_SUFFIXES libgsf-1
   )

FIND_LIBRARY(LIBGSF_LIBRARIES NAMES gsf-1 libgsf-1
   HINTS
   ${PC_LIBGSF_LIBDIR}
   ${PC_LIBGSF_LIBRARY_DIRS}
   )

FIND_PROGRAM(LIBGSF_GSF_EXECUTABLE gsf)
FIND_PROGRAM(LIBGSF_GSFOFFICETHUMBNAILER_EXECUTABLE gsf-office-thumbnailer)
FIND_PROGRAM(LIBGSF_GSFVBADUMP_EXECUTABLE gsf-vba-dump)

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBGSF_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBGSF DEFAULT_MSG LIBGSF_LIBRARIES LIBGSF_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBGSF_INCLUDE_DIR LIBGSF_LIBRARIES LIBGSF_GSF_EXECUTABLE LIBGSF_GSFOFFICETHUMBNAILER_EXECUTABLE LIBGSF_GSFVBADUMP_EXECUTABLE )

