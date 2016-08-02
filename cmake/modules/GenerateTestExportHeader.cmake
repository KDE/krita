#.rst:
# GenerateTestExportHeader
# --------------------
#
# This module provides the function GENERATE_TEST_EXPORT_HEADER().
# It is almost identical to generate_export_header() but produces
# additional test export symbols using exporttestheader.cmake.in
#
# The standard format is ${BASE_NAME}_TEST_EXPORT
# If you use a custom export symbol like EXPORTMYLIB
# the test symbol will be of the format EXPORTMYLIB_TEST
#

#=============================================================================
# Copyright 2011 Stephen Kelly <steveire@gmail.com>
# Copyright 2015 Michael Abrahams <miabraha@gmail.com>
#
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

include(CMakeParseArguments)
include(CheckCXXCompilerFlag)

# TODO: Install this macro separately?
macro(_check_cxx_compiler_attribute _ATTRIBUTE _RESULT)
  check_cxx_source_compiles("${_ATTRIBUTE} int somefunc() { return 0; }
    int main() { return somefunc();}" ${_RESULT}
  )
endmacro()

macro(_test_compiler_hidden_visibility)

  if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.2")
    set(GCC_TOO_OLD TRUE)
  elseif(CMAKE_COMPILER_IS_GNUC AND CMAKE_C_COMPILER_VERSION VERSION_LESS "4.2")
    set(GCC_TOO_OLD TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES Intel AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
    set(_INTEL_TOO_OLD TRUE)
  endif()

  # Exclude XL here because it misinterprets -fvisibility=hidden even though
  # the check_cxx_compiler_flag passes
  if(NOT GCC_TOO_OLD
      AND NOT _INTEL_TOO_OLD
      AND NOT WIN32
      AND NOT CYGWIN
      AND NOT CMAKE_CXX_COMPILER_ID MATCHES XL
      AND NOT CMAKE_CXX_COMPILER_ID MATCHES PGI
      AND NOT CMAKE_CXX_COMPILER_ID MATCHES Watcom)
    check_cxx_compiler_flag(-fvisibility=hidden COMPILER_HAS_HIDDEN_VISIBILITY)
    check_cxx_compiler_flag(-fvisibility-inlines-hidden
      COMPILER_HAS_HIDDEN_INLINE_VISIBILITY)
    option(USE_COMPILER_HIDDEN_VISIBILITY
      "Use HIDDEN visibility support if available." ON)
    mark_as_advanced(USE_COMPILER_HIDDEN_VISIBILITY)
  endif()
endmacro()

macro(_test_compiler_has_deprecated)
  if(CMAKE_CXX_COMPILER_ID MATCHES Borland
      OR CMAKE_CXX_COMPILER_ID MATCHES HP
      OR GCC_TOO_OLD
      OR CMAKE_CXX_COMPILER_ID MATCHES PGI
      OR CMAKE_CXX_COMPILER_ID MATCHES Watcom)
    set(COMPILER_HAS_DEPRECATED "" CACHE INTERNAL
      "Compiler support for a deprecated attribute")
  else()
    _check_cxx_compiler_attribute("__attribute__((__deprecated__))"
      COMPILER_HAS_DEPRECATED_ATTR)
    if(COMPILER_HAS_DEPRECATED_ATTR)
      set(COMPILER_HAS_DEPRECATED "${COMPILER_HAS_DEPRECATED_ATTR}"
        CACHE INTERNAL "Compiler support for a deprecated attribute")
    else()
      _check_cxx_compiler_attribute("__declspec(deprecated)"
        COMPILER_HAS_DEPRECATED)
    endif()
  endif()
endmacro()

get_filename_component(_GENERATE_TEST_EXPORT_HEADER_MODULE_DIR
  "${CMAKE_CURRENT_LIST_FILE}" PATH)

macro(_DO_SET_MACRO_VALUES TARGET_LIBRARY)
  set(DEFINE_DEPRECATED)
  set(DEFINE_EXPORT)
  set(DEFINE_IMPORT)
  set(DEFINE_NO_EXPORT)

  if (COMPILER_HAS_DEPRECATED_ATTR)
    set(DEFINE_DEPRECATED "__attribute__ ((__deprecated__))")
  elseif(COMPILER_HAS_DEPRECATED)
    set(DEFINE_DEPRECATED "__declspec(deprecated)")
  endif()

  get_property(type TARGET ${TARGET_LIBRARY} PROPERTY TYPE)

  if(NOT ${type} STREQUAL "STATIC_LIBRARY")
    if(WIN32)
      set(DEFINE_EXPORT "__declspec(dllexport)")
      set(DEFINE_IMPORT "__declspec(dllimport)")
    elseif(COMPILER_HAS_HIDDEN_VISIBILITY AND USE_COMPILER_HIDDEN_VISIBILITY)
      set(DEFINE_EXPORT "__attribute__((visibility(\"default\")))")
      set(DEFINE_IMPORT "__attribute__((visibility(\"default\")))")
      set(DEFINE_NO_EXPORT "__attribute__((visibility(\"hidden\")))")
    endif()
  endif()
endmacro()

macro(_DO_GENERATE_TEST_EXPORT_HEADER TARGET_LIBRARY)
  # Option overrides
  set(options DEFINE_NO_DEPRECATED)
  set(oneValueArgs PREFIX_NAME BASE_NAME EXPORT_MACRO_NAME EXPORT_FILE_NAME
    DEPRECATED_MACRO_NAME NO_EXPORT_MACRO_NAME STATIC_DEFINE
    NO_DEPRECATED_MACRO_NAME)
  set(multiValueArgs)

  cmake_parse_arguments(_GEH "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN})

  set(BASE_NAME "${TARGET_LIBRARY}")

  if(_GEH_BASE_NAME)
    set(BASE_NAME ${_GEH_BASE_NAME})
  endif()

  string(TOUPPER ${BASE_NAME} BASE_NAME_UPPER)
  string(TOLOWER ${BASE_NAME} BASE_NAME_LOWER)

  # Default options
  set(EXPORT_MACRO_NAME "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_EXPORT")
  set(EXPORT_TEST_MACRO_NAME "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_TEST_EXPORT")
  set(NO_EXPORT_MACRO_NAME "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_NO_EXPORT")
  set(EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME_LOWER}_export.h")
  set(DEPRECATED_MACRO_NAME "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_DEPRECATED")
  set(STATIC_DEFINE "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_STATIC_DEFINE")
  set(NO_DEPRECATED_MACRO_NAME
    "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_NO_DEPRECATED")

  if(_GEH_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to GENERATE_TEST_EXPORT_HEADER(): \"${_GEH_UNPARSED_ARGUMENTS}\"")
  endif()

  if(_GEH_EXPORT_MACRO_NAME)
    set(EXPORT_MACRO_NAME ${_GEH_PREFIX_NAME}${_GEH_EXPORT_MACRO_NAME})
    set(EXPORT_TEST_MACRO_NAME "${_GEH_PREFIX_NAME}${_GEH_EXPORT_MACRO_NAME}_TEST")
  endif()
  string(MAKE_C_IDENTIFIER ${EXPORT_MACRO_NAME} EXPORT_MACRO_NAME)
  string(MAKE_C_IDENTIFIER ${EXPORT_MACRO_NAME} EXPORT_TEST_MACRO_NAME)
  if(_GEH_EXPORT_FILE_NAME)
    if(IS_ABSOLUTE ${_GEH_EXPORT_FILE_NAME})
      set(EXPORT_FILE_NAME ${_GEH_EXPORT_FILE_NAME})
    else()
      set(EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${_GEH_EXPORT_FILE_NAME}")
    endif()
  endif()
  if(_GEH_DEPRECATED_MACRO_NAME)
    set(DEPRECATED_MACRO_NAME ${_GEH_PREFIX_NAME}${_GEH_DEPRECATED_MACRO_NAME})
  endif()
  string(MAKE_C_IDENTIFIER ${DEPRECATED_MACRO_NAME} DEPRECATED_MACRO_NAME)
  if(_GEH_NO_EXPORT_MACRO_NAME)
    set(NO_EXPORT_MACRO_NAME ${_GEH_PREFIX_NAME}${_GEH_NO_EXPORT_MACRO_NAME})
  endif()
  string(MAKE_C_IDENTIFIER ${NO_EXPORT_MACRO_NAME} NO_EXPORT_MACRO_NAME)
  if(_GEH_STATIC_DEFINE)
    set(STATIC_DEFINE ${_GEH_PREFIX_NAME}${_GEH_STATIC_DEFINE})
  endif()
  string(MAKE_C_IDENTIFIER ${STATIC_DEFINE} STATIC_DEFINE)

  if(_GEH_DEFINE_NO_DEPRECATED)
    set(DEFINE_NO_DEPRECATED TRUE)
  endif()

  if(_GEH_NO_DEPRECATED_MACRO_NAME)
    set(NO_DEPRECATED_MACRO_NAME
      ${_GEH_PREFIX_NAME}${_GEH_NO_DEPRECATED_MACRO_NAME})
  endif()
  string(MAKE_C_IDENTIFIER ${NO_DEPRECATED_MACRO_NAME} NO_DEPRECATED_MACRO_NAME)

  set(INCLUDE_GUARD_NAME "${EXPORT_MACRO_NAME}_H")

  get_target_property(EXPORT_IMPORT_CONDITION ${TARGET_LIBRARY} DEFINE_SYMBOL)

  if(NOT EXPORT_IMPORT_CONDITION)
    set(EXPORT_IMPORT_CONDITION ${TARGET_LIBRARY}_EXPORTS)
  endif()
  string(MAKE_C_IDENTIFIER ${EXPORT_IMPORT_CONDITION} EXPORT_IMPORT_CONDITION)

  configure_file("${_GENERATE_TEST_EXPORT_HEADER_MODULE_DIR}/exporttestheader.cmake.in"
    "${EXPORT_FILE_NAME}" @ONLY)
endmacro()

function(GENERATE_TEST_EXPORT_HEADER TARGET_LIBRARY)
  get_property(type TARGET ${TARGET_LIBRARY} PROPERTY TYPE)
  if(NOT ${type} STREQUAL "STATIC_LIBRARY"
      AND NOT ${type} STREQUAL "SHARED_LIBRARY"
      AND NOT ${type} STREQUAL "OBJECT_LIBRARY"
      AND NOT ${type} STREQUAL "MODULE_LIBRARY")
    message(WARNING "This macro can only be used with libraries")
    return()
  endif()
  _test_compiler_hidden_visibility()
  _test_compiler_has_deprecated()
  _do_set_macro_values(${TARGET_LIBRARY})
  _do_generate_test_export_header(${TARGET_LIBRARY} ${ARGN})
endfunction()

function(add_compiler_export_flags)
  if(NOT CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 2.8.12)
    message(DEPRECATION "The add_compiler_export_flags function is obsolete. Use the CXX_VISIBILITY_PRESET and VISIBILITY_INLINES_HIDDEN target properties instead.")
  endif()

  _test_compiler_hidden_visibility()
  _test_compiler_has_deprecated()

  if(NOT (USE_COMPILER_HIDDEN_VISIBILITY AND COMPILER_HAS_HIDDEN_VISIBILITY))
    # Just return if there are no flags to add.
    return()
  endif()

  set (EXTRA_FLAGS "-fvisibility=hidden")

  if(COMPILER_HAS_HIDDEN_INLINE_VISIBILITY)
    set (EXTRA_FLAGS "${EXTRA_FLAGS} -fvisibility-inlines-hidden")
  endif()

  # Either return the extra flags needed in the supplied argument, or to the
  # CMAKE_CXX_FLAGS if no argument is supplied.
  if(ARGC GREATER 0)
    set(${ARGV0} "${EXTRA_FLAGS}" PARENT_SCOPE)
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_FLAGS}" PARENT_SCOPE)
  endif()
endfunction()
