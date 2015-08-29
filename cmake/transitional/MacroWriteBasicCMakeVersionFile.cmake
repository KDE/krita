#  MACRO_WRITE_BASIC_CMAKE_VERSION_FILE( _filename _major _minor _patch)
#    Writes a file for use as <package>ConfigVersion.cmake file to <_filename>.
#    See the documentation of FIND_PACKAGE() for details on this.
#    _filename is the output filename, it should be in the build tree.
#    _major is the major version number of the project to be installed
#    _minor is the minor version number of the project to be installed
#    _patch is the patch version number of the project to be installed
#

# Copyright (c) 2008, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

get_filename_component(_currentListFileDir ${CMAKE_CURRENT_LIST_FILE} PATH)

function(MACRO_WRITE_BASIC_CMAKE_VERSION_FILE _filename _major _minor _patch)
   set(PROJECT_VERSION_MAJOR ${_major})
   set(PROJECT_VERSION_MINOR ${_minor})
   set(PROJECT_VERSION_PATCH ${_patch})
   configure_file(${_currentListFileDir}/BasicFindPackageVersion.cmake.in "${_filename}" @ONLY)
endfunction(MACRO_WRITE_BASIC_CMAKE_VERSION_FILE _major _minor _patch)
