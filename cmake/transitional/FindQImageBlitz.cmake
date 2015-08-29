# - Try to find the qimageblitz lib
# Once done this will define
#
#  QIMAGEBLITZ_FOUND - system has qimageblitz lib
#  QIMAGEBLITZ_INCLUDES - the qimageblitz include directory
#  QIMAGEBLITZ_LIBRARIES - The libraries needed to use qimageblitz

# Copyright (c) 2006, Montel Laurent, <montel@kde.org>
# Copyright (c) 2007, Allen Winter, <winter@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindLibraryWithDebug)

if (QIMAGEBLITZ_INCLUDES AND QIMAGEBLITZ_LIBRARIES)
  set(QImageBlitz_FIND_QUIETLY TRUE)
endif (QIMAGEBLITZ_INCLUDES AND QIMAGEBLITZ_LIBRARIES)

if (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_QIMAGEBLITZ QUIET qimageblitz)
endif (NOT WIN32)

find_path(QIMAGEBLITZ_INCLUDES
  NAMES
  qimageblitz.h
  PATH_SUFFIXES qimageblitz
  HINTS
  $ENV{QIMAGEBLITZDIR}/include
  ${PC_QIMAGEBLITZ_INCLUDEDIR}
  ${KDE4_INCLUDE_DIR}
  ${INCLUDE_INSTALL_DIR}
)

find_library_with_debug(QIMAGEBLITZ_LIBRARIES
  WIN32_DEBUG_POSTFIX d
  qimageblitz
  HINTS
  $ENV{QIMAGEBLITZDIR}/lib
  ${PC_QIMAGEBLITZ_LIBDIR}
  ${KDE4_LIB_DIR}
  ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QImageBlitz DEFAULT_MSG 
                                  QIMAGEBLITZ_INCLUDES QIMAGEBLITZ_LIBRARIES)

mark_as_advanced(QIMAGEBLITZ_INCLUDES QIMAGEBLITZ_LIBRARIES)
