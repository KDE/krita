#
# SPDX-License-Identifier: BSD-3-Clause
#
# Always include srcdir and builddir in include path
# This saves typing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} in about every subdir
# since cmake 2.4.0
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are preferred over the already installed ones
# since cmake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# define the generic version of the libraries here
# this makes it easy to advance it when the next KDE release comes
# Use this version number for libraries which are at version n in KDE version n
set(GENERIC_LIB_VERSION "4.11.0")
set(GENERIC_LIB_SOVERSION "4")

# Use this version number for libraries which are already at version n+1 in KDE version n
set(KDE_NON_GENERIC_LIB_VERSION "5.11.0")
set(KDE_NON_GENERIC_LIB_SOVERSION "5")

# windows does not support LD_LIBRARY_PATH or similar
# all searchable directories has to be defined by the PATH environment var
# to reduce the number of required paths executables are placed into
# the build bin dir
if (WIN32)
 set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
  if (MINGW)
      set (CMAKE_RC_COMPILER_INIT windres)
      enable_language (RC)
      set (CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")
  endif(MINGW)
endif(WIN32)
