#
# Find an installation of Soprano
#
# Sets the following variables:
#  Soprano_FOUND, SOPRANO_FOUND  - true is Soprano has been found
#  SOPRANO_ONTO2VOCABULARYCLASS_EXECUTABLE - the onto2vocabularyclass program, required for adding ontologies
#  SOPRANO_SOPRANOCMD_EXECUTABLE - the sopranocmd program
#  SOPRANO_INCLUDE_DIR      - The include directory
#  SOPRANO_LIBRARIES        - The Soprano core library to link to (libsoprano)
#  SOPRANO_INDEX_LIBRARIES  - The Soprano index library (libsopranoindex)
#  SOPRANO_CLIENT_LIBRARIES - The Soprano client library (libsopranoclient)
#  SOPRANO_SERVER_LIBRARIES - The Soprano server library (libsopranoserver)
#  SOPRANO_VERSION          - The Soprano version (string value)
#
# SOPRANO_PLUGIN_NQUADPARSER_FOUND      - true if the nquadparser plugin is found
# SOPRANO_PLUGIN_NQUADSERIALIZER_FOUND  - true if the nquadserializer plugin is found
# SOPRANO_PLUGIN_RAPTORPARSER_FOUND     - true if the raptorparser plugin is found
# SOPRANO_PLUGIN_RAPTORSERIALIZER_FOUND - true if the raptorserializer plugin is found
# SOPRANO_PLUGIN_REDLANDBACKEND_FOUND   - true if the redlandbackend plugin is found
# SOPRANO_PLUGIN_SESAME2BACKEND_FOUND   - true if the sesame2backend plugin is found
# SOPRANO_PLUGIN_VIRTUOSOBACKEND_FOUND  - true if the virtuosobackend plugin is found 
#
# Options:
#  Set SOPRANO_MIN_VERSION to set the minimum required Soprano version (default: 1.99)
#
# FindSoprano.cmake supports the COMPONENTS keyword of find_package().
# If the REQUIRED keyword is used and any of the specified components have not been
# found, SOPRANO_FOUND will be set to FALSE.
#
# The following components are supported:
#   PLUGIN_NQUADPARSER
#   PLUGIN_NQUADSERIALIZER
#   PLUGIN_RAPTORPARSER
#   PLUGIN_RAPTORSERIALIZER
#   PLUGIN_REDLANDBACKEND
#   PLUGIN_SESAME2BACKEND
#   PLUGIN_VIRTUOSOBACKEND

# Copyright (c) 2008, Sebastian Trueg, <sebastian@trueg.de>
# Copyright (c) 2009, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


include(FindLibraryWithDebug)

find_program(SOPRANO_SOPRANOCMD_EXECUTABLE
  NAMES sopranocmd 
  HINTS
  ${BIN_INSTALL_DIR}
  ${KDE4_BIN_INSTALL_DIR}
  )

if(NOT WINCE)
find_program(SOPRANO_ONTO2VOCABULARYCLASS_EXECUTABLE
  NAMES onto2vocabularyclass
  HINTS
  ${BIN_INSTALL_DIR}
  ${KDE4_BIN_INSTALL_DIR}
  )
else(NOT WINCE)
find_program(SOPRANO_ONTO2VOCABULARYCLASS_EXECUTABLE
  NAMES onto2vocabularyclass
  PATHS ${HOST_BINDIR}
  NO_DEFAULT_PATH
  )
endif(NOT WINCE)


find_path(SOPRANO_INCLUDE_DIR 
  NAMES
  soprano/soprano.h
  HINTS
  ${INCLUDE_INSTALL_DIR}
  ${KDE4_INCLUDE_DIR}
  )

find_library_with_debug(SOPRANO_INDEX_LIBRARIES 
  WIN32_DEBUG_POSTFIX d
  NAMES
  sopranoindex
  HINTS
  ${LIB_INSTALL_DIR}
  ${KDE4_LIB_DIR}
  )

find_library_with_debug(SOPRANO_CLIENT_LIBRARIES 
  WIN32_DEBUG_POSTFIX d
  NAMES
  sopranoclient
  HINTS
  ${LIB_INSTALL_DIR}
  ${KDE4_LIB_DIR}
  )

find_library_with_debug(SOPRANO_LIBRARIES
  WIN32_DEBUG_POSTFIX d
  NAMES soprano
  HINTS
  ${LIB_INSTALL_DIR}
  ${KDE4_LIB_DIR}
)

find_library_with_debug(SOPRANO_SERVER_LIBRARIES 
  WIN32_DEBUG_POSTFIX d
  NAMES
  sopranoserver
  HINTS
  ${LIB_INSTALL_DIR}
  ${KDE4_LIB_DIR}
  )

# check Soprano version

# Support SOPRANO_MIN_VERSION for compatibility:
if(NOT Soprano_FIND_VERSION)
  set(Soprano_FIND_VERSION "${SOPRANO_MIN_VERSION}")
endif(NOT Soprano_FIND_VERSION)

# We set a default for the minimum required version to be backwards compatible
if(NOT Soprano_FIND_VERSION)
  set(Soprano_FIND_VERSION "1.99")
endif(NOT Soprano_FIND_VERSION)


if(SOPRANO_INCLUDE_DIR)
  file(READ ${SOPRANO_INCLUDE_DIR}/soprano/version.h SOPRANO_VERSION_CONTENT)
  string(REGEX MATCH "SOPRANO_VERSION_STRING \".*\"\n" SOPRANO_VERSION_MATCH "${SOPRANO_VERSION_CONTENT}")
  if(SOPRANO_VERSION_MATCH)
    string(REGEX REPLACE "SOPRANO_VERSION_STRING \"(.*)\"\n" "\\1" SOPRANO_VERSION ${SOPRANO_VERSION_MATCH})
    # find_package_handle_standard_args will do the version checking
  endif(SOPRANO_VERSION_MATCH)
endif(SOPRANO_INCLUDE_DIR)

set(_SOPRANO_REQUIRED_COMPONENTS_RESULTS)
if( Soprano_FIND_COMPONENTS )
  foreach( _component ${Soprano_FIND_COMPONENTS} )
    set(_SOPRANO_REQUIRED_COMPONENTS_RESULTS ${_SOPRANO_REQUIRED_COMPONENTS_RESULTS} SOPRANO_${_component}_FOUND)
  endforeach( _component )
endif( Soprano_FIND_COMPONENTS )

#look for parser plugins
if(SOPRANO_INCLUDE_DIR)
  get_filename_component(_SOPRANO_PREFIX ${SOPRANO_INCLUDE_DIR} PATH)

  find_path(SOPRANO_PLUGIN_ROOT_DIR 
    NAMES
    soprano/plugins
    HINTS
    ${_SOPRANO_PREFIX}/share
    ${SHARE_INSTALL_PREFIX} 
    PATH_SUFFIXES share
    )
  set(SOPRANO_PLUGIN_DIR "${SOPRANO_PLUGIN_ROOT_DIR}/soprano/plugins")

  if(EXISTS ${SOPRANO_PLUGIN_DIR}/nquadparser.desktop)
    set(SOPRANO_PLUGIN_NQUADPARSER_FOUND TRUE)
    set(_plugins "${_plugins} nquadparser")
  endif(EXISTS ${SOPRANO_PLUGIN_DIR}/nquadparser.desktop)

  if(EXISTS ${SOPRANO_PLUGIN_DIR}/nquadserializer.desktop)
    set(SOPRANO_PLUGIN_NQUADSERIALIZER_FOUND TRUE)
    set(_plugins "${_plugins} nquadserializer")
  endif(EXISTS ${SOPRANO_PLUGIN_DIR}/nquadserializer.desktop)

  if(EXISTS ${SOPRANO_PLUGIN_DIR}/raptorparser.desktop)
    set(SOPRANO_PLUGIN_RAPTORPARSER_FOUND TRUE)
    set(_plugins "${_plugins} raptorparser")
  endif(EXISTS ${SOPRANO_PLUGIN_DIR}/raptorparser.desktop)

  if(EXISTS ${SOPRANO_PLUGIN_DIR}/raptorserializer.desktop)
    set(SOPRANO_PLUGIN_RAPTORSERIALIZER_FOUND TRUE)
    set(_plugins "${_plugins} raptorserializer")
  endif(EXISTS ${SOPRANO_PLUGIN_DIR}/raptorserializer.desktop)

  if(EXISTS ${SOPRANO_PLUGIN_DIR}/redlandbackend.desktop)
    set(SOPRANO_PLUGIN_REDLANDBACKEND_FOUND TRUE)
    set(_plugins "${_plugins} redlandbackend")
  endif(EXISTS ${SOPRANO_PLUGIN_DIR}/redlandbackend.desktop)

  if(EXISTS ${SOPRANO_PLUGIN_DIR}/sesame2backend.desktop)
    set(SOPRANO_PLUGIN_SESAME2BACKEND_FOUND TRUE)
    set(_plugins "${_plugins} sesame2backend")
  endif(EXISTS ${SOPRANO_PLUGIN_DIR}/sesame2backend.desktop)

  if(EXISTS ${SOPRANO_PLUGIN_DIR}/virtuosobackend.desktop)
    set(SOPRANO_PLUGIN_VIRTUOSOBACKEND_FOUND TRUE)
    set(_plugins "${_plugins} virtuosobackend")
  endif(EXISTS ${SOPRANO_PLUGIN_DIR}/virtuosobackend.desktop)

  # make sure the Soprano cmake macros are found
  # We also include it directly for convenience
  find_file(_SOPRANO_MACRO_FILE NAMES SopranoAddOntology.cmake HINTS ${_SOPRANO_PREFIX}/share/soprano/cmake )
  if(_SOPRANO_MACRO_FILE)
    # new Soprano > 2.3.0 location
    include(${_SOPRANO_MACRO_FILE})
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${_SOPRANO_PREFIX}/share/soprano/cmake)
  else(_SOPRANO_MACRO_FILE)
    # the old Soprano 2.3.0 location
    find_file(_SOPRANO_MACRO_FILE_OLD NAMES SopranoAddOntology.cmake HINTS ${_SOPRANO_PREFIX}/share/apps/cmake/modules )
    if(_SOPRANO_MACRO_FILE_OLD)
      include(${_SOPRANO_MACRO_FILE_OLD})
      set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${_SOPRANO_PREFIX}/share/apps/cmake/modules)
    endif(_SOPRANO_MACRO_FILE_OLD)
  endif(_SOPRANO_MACRO_FILE)

endif(SOPRANO_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Soprano REQUIRED_VARS SOPRANO_INCLUDE_DIR SOPRANO_LIBRARIES
                                                       ${_SOPRANO_REQUIRED_COMPONENTS_RESULTS} 
                                         VERSION_VAR SOPRANO_VERSION)

# for compatibility:
set(Soprano_FOUND ${SOPRANO_FOUND})

# check for all the libs as required to make sure that we do not try to compile with an old version

if(SOPRANO_FOUND AND SOPRANO_INDEX_LIBRARIES)
  set(SopranoIndex_FOUND TRUE)
endif(SOPRANO_FOUND AND SOPRANO_INDEX_LIBRARIES)

if(SOPRANO_FOUND AND SOPRANO_CLIENT_LIBRARIES)
  set(SopranoClient_FOUND TRUE)
endif(SOPRANO_FOUND AND SOPRANO_CLIENT_LIBRARIES)

if(SOPRANO_FOUND AND SOPRANO_SERVER_LIBRARIES)
  set(SopranoServer_FOUND TRUE)
endif(SOPRANO_FOUND AND SOPRANO_SERVER_LIBRARIES)



mark_as_advanced(SOPRANO_CLIENT_LIBRARIES 
                 SOPRANO_INDEX_LIBRARIES
                 SOPRANO_LIBRARIES
                 SOPRANO_SERVER_LIBRARIES
                 SOPRANO_INCLUDE_DIR
                 SOPRANO_PLUGIN_ROOT_DIR
                 _SOPRANO_MACRO_FILE
                 SOPRANO_ONTO2VOCABULARYCLASS_EXECUTABLE
                 SOPRANO_SOPRANOCMD_EXECUTABLE
                 )

