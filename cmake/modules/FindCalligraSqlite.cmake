# - Find SQLite 3
# This module can be used to find SQLite 3.
# (THIS IS AN EXTENDED VERSION OF THE FUTURE FindSqlite.cmake FOR CALLIGRA TO MAKE SURE
#  THE DETECTION WORKS AS EXPECTED ON KDE >= 4.3)
#
# Accepted variables:
#  SQLITE_RECOMMENDED_VERSION
#        If defined, warning will be displayed for SQLite versions older
#        than specified.
#        For example use this before calling find_package:
#           set(SQLITE_RECOMMENDED_VERSION "3.6.22")
#  SQLITE_LOAD_EXTENSION_REQUIRED
#        If defined, extensions loading feature of SQLite is required.
#        For example use this before calling find_package:
#           set(SQLITE_LOAD_EXTENSION_REQUIRED ON)
#
# This module allows to depend on a particular minimum version of SQLite.
# To acomplish that one should use the apropriate cmake syntax for
# find_package(). For example to depend on SQLite >= 3.6.16 one should use:
#
#  find_package(SQLite 3.6.16 REQUIRED)
#
# Variables that FindSqlite.cmake sets:
#  SQLITE_FOUND             TRUE if required version of Sqlite has been found
#  SQLITE_INCLUDE_DIR       the SQLite's include directory
#  SQLITE_LIBRARIES         link these to use SQLite
#  SQLITE_MIN_VERSION       minimum version, if as the second argument of find_package()
#  SQLITE_VERSION_STRING    found version of SQLite, e.g. "3.6.16"
#  SQLITE_VERSION           integer for the found version of SQLite, e.g. 3006016 for 3.6.16
#  SQLITE_MIN_VERSION_MAJOR found major version of SQLite, e.g. 3
#  SQLITE_MIN_VERSION_MINOR found major version of SQLite, e.g. 6
#  SQLITE_MIN_VERSION_PATCH found major version of SQLite, e.g. 16
#  SQLITE_SHELL             sqlite3 executable with full path
#  SQLITE_COMPILE_OPTIONS   compile options used to build libsqlite3
#  SQLITE_CHECK_COMPILE_OPTION(X)
#      Sets X variable to ON if option X has been set in compile options for libsqlite3.
#      Supported can be, among others:
#      ENABLE_COLUMN_METADATA ENABLE_FTS3 ENABLE_FTS3_PARENTHESIS ENABLE_MEMORY_MANAGEMENT
#      ENABLE_RTREE ENABLE_UNLOCK_NOTIFY ENABLE_UPDATE_DELETE_LIMIT SECURE_DELETE SOUNDEX
#      TEMP_STORE THREADSAFE OMIT_LOAD_EXTENSION.
#      For complete list of options read http://www.sqlite.org/compile.html
#      SQLITE_COMPILE_OPTIONS list is used for the check.
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Copyright (C) 2008 Gilles Caulier <caulier.gilles@gmail.com>
# Copyright (C) 2010-2011 Jarosław Staniek <staniek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES)
   if(SQLITE_LOAD_EXTENSION_REQUIRED AND SQLITE_LOAD_EXTENSION OR NOT SQLITE_LOAD_EXTENSION_REQUIRED)
      # in cache already
      SET(Sqlite_FIND_QUIETLY TRUE)
   endif(SQLITE_LOAD_EXTENSION_REQUIRED AND SQLITE_LOAD_EXTENSION OR NOT SQLITE_LOAD_EXTENSION_REQUIRED)
endif(SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if(NOT WIN32)
  find_package(PkgConfig)

  pkg_check_modules(PC_SQLITE QUIET sqlite3)

  set(SQLITE_DEFINITIONS ${PC_SQLITE_CFLAGS_OTHER})
endif(NOT WIN32)

FIND_PATH(SQLITE_INCLUDE_DIR NAMES sqlite3.h
  PATHS
  ${PC_SQLITE_INCLUDEDIR}
  ${PC_SQLITE_INCLUDE_DIRS}
)

FIND_LIBRARY(SQLITE_LIBRARIES NAMES sqlite3
  PATHS
  ${PC_SQLITE_LIBDIR}
  ${PC_SQLITE_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sqlite DEFAULT_MSG SQLITE_INCLUDE_DIR SQLITE_LIBRARIES)

macro(_check_min_sqlite_version)
    # Suppport finding at least a particular version, for instance FIND_PACKAGE(Sqlite 3.6.22)
    if(Sqlite_FIND_VERSION)
        set(SQLITE_MIN_VERSION ${Sqlite_FIND_VERSION} CACHE STRING "Required SQLite minimal version" FORCE)
        set(SQLITE_MIN_VERSION_MAJOR ${Sqlite_FIND_VERSION_MAJOR} CACHE STRING "Required SQLite minimal version (major)" FORCE)
        set(SQLITE_MIN_VERSION_MINOR ${Sqlite_FIND_VERSION_MINOR} CACHE STRING "Required SQLite minimal version (minor)" FORCE)
        set(SQLITE_MIN_VERSION_PATCH ${Sqlite_FIND_VERSION_PATCH} CACHE STRING "Required SQLite minimal version (patch)" FORCE)
        math(EXPR SQLITE_MIN_VERSION_NUMBER
            "${SQLITE_MIN_VERSION_MAJOR} * 1000000 + ${SQLITE_MIN_VERSION_MINOR} * 1000 + ${SQLITE_MIN_VERSION_PATCH}")
        if(SQLITE_MIN_VERSION_NUMBER GREATER SQLITE_VERSION)
            if(Sqlite_FIND_REQUIRED)
                message(FATAL_ERROR "Minimal SQLite version required: ${SQLITE_MIN_VERSION}, found ${SQLITE_VERSION_STRING}")
            else(Sqlite_FIND_REQUIRED)
                message(STATUS "WARNING: Minimal SQLite version required: ${SQLITE_MIN_VERSION}, found ${SQLITE_VERSION_STRING}")
            endif(Sqlite_FIND_REQUIRED)
            unset(SQLITE_FOUND)
            unset(SQLITE_FOUND CACHE)
        endif(SQLITE_MIN_VERSION_NUMBER GREATER SQLITE_VERSION)
    endif(Sqlite_FIND_VERSION)

    if(SQLITE_FOUND)
        if(NOT Sqlite_FIND_QUIETLY)
            message(STATUS "Found SQLite version ${SQLITE_VERSION_STRING}")
        endif(NOT Sqlite_FIND_QUIETLY)
    endif(SQLITE_FOUND)
endmacro(_check_min_sqlite_version)

macro(_check_recommended_sqlite_version)
    if(SQLITE_RECOMMENDED_VERSION)
        string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1" SQLITE_RECOMMENDED_VERSION_MAJOR ${SQLITE_RECOMMENDED_VERSION})
        string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\2" SQLITE_RECOMMENDED_VERSION_MINOR ${SQLITE_RECOMMENDED_VERSION})
        string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\3" SQLITE_RECOMMENDED_VERSION_PATCH ${SQLITE_RECOMMENDED_VERSION})
        math(EXPR SQLITE_RECOMMENDED_VERSION_NUMBER
            "${SQLITE_RECOMMENDED_VERSION_MAJOR} * 1000000 + ${SQLITE_RECOMMENDED_VERSION_MINOR} * 1000 + ${SQLITE_RECOMMENDED_VERSION_PATCH}")
        if(SQLITE_RECOMMENDED_VERSION_NUMBER GREATER SQLITE_VERSION)
            message(STATUS "WARNING: Recommended SQLite version is ${SQLITE_RECOMMENDED_VERSION}")
        endif(SQLITE_RECOMMENDED_VERSION_NUMBER GREATER SQLITE_VERSION)
    endif(SQLITE_RECOMMENDED_VERSION)
endmacro(_check_recommended_sqlite_version)

macro(_get_compile_options)
    FIND_PROGRAM(SQLITE_SHELL sqlite3 DOC "sqlite3 executable with full path")
    if (SQLITE_SHELL)
        SET(_COMPILE_OPTIONS_TEST_DB pragma_compile_options_test.sqlite3)
        EXECUTE_PROCESS(COMMAND ${SQLITE_SHELL} ${_COMPILE_OPTIONS_TEST_DB} "pragma compile_options"
                        OUTPUT_VARIABLE _COMPILE_OPTIONS_RESULT)
        STRING(REGEX REPLACE "=1\n" ";" SQLITE_COMPILE_OPTIONS "${_COMPILE_OPTIONS_RESULT}")
        STRING(REPLACE "\n" ";" SQLITE_COMPILE_OPTIONS "${SQLITE_COMPILE_OPTIONS}")
        SET(SQLITE_COMPILE_OPTIONS ${SQLITE_COMPILE_OPTIONS})
    else (SQLITE_SHELL)
    endif (SQLITE_SHELL)
endmacro(_get_compile_options)

macro(SQLITE_CHECK_COMPILE_OPTION _OPTION_NAME)
    LIST(FIND SQLITE_COMPILE_OPTIONS "${_OPTION_NAME}" _RESULT)
    if (NOT ${_RESULT} EQUAL -1)
        SET(${_OPTION_NAME} ON)
    endif (NOT ${_RESULT} EQUAL -1)
endmacro(SQLITE_CHECK_COMPILE_OPTION)

if(NOT EXISTS "${SQLITE_INCLUDE_DIR}/sqlite3.h")
    unset(SQLITE_FOUND)
    unset(SQLITE_FOUND CACHE)
endif(NOT EXISTS "${SQLITE_INCLUDE_DIR}/sqlite3.h")

if(SQLITE_FOUND)
   file(READ "${SQLITE_INCLUDE_DIR}/sqlite3.h" SQLITE_VERSION_CONTENT)
   string(REGEX MATCH "#define SQLITE_VERSION[ ]*\"[0-9]+\\.[0-9]+\\.[0-9]+\".*\n" SQLITE_VERSION_STRING_MATCH ${SQLITE_VERSION_CONTENT})
   string(REGEX MATCH "#define SQLITE_VERSION_NUMBER[ ]*[0-9]*\n" SQLITE_VERSION_MATCH ${SQLITE_VERSION_CONTENT})
   if(SQLITE_VERSION_STRING_MATCH AND SQLITE_VERSION_MATCH)
      string(REGEX REPLACE "#define SQLITE_VERSION[ ]*\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*\n" "\\1" SQLITE_VERSION_STRING ${SQLITE_VERSION_STRING_MATCH})
      string(REGEX REPLACE "#define SQLITE_VERSION_NUMBER[ ]*([0-9]*)\n" "\\1" SQLITE_VERSION ${SQLITE_VERSION_MATCH})
      set(SQLITE_VERSION ${SQLITE_VERSION} CACHE STRING "SQLite numeric version")
      set(SQLITE_VERSION_STRING ${SQLITE_VERSION_STRING} CACHE STRING "SQLite version")
      _check_min_sqlite_version()
      _check_recommended_sqlite_version()
   endif(SQLITE_VERSION_STRING_MATCH AND SQLITE_VERSION_MATCH)
   _get_compile_options()
   if(SQLITE_LOAD_EXTENSION_REQUIRED)
      SQLITE_CHECK_COMPILE_OPTION(OMIT_LOAD_EXTENSION)
      if(OMIT_LOAD_EXTENSION)
          message(STATUS "WARNING: SQLite found but it is not built with support for extensions loading. It should be configured with --enable-load-extension option.")
          unset(SQLITE_FOUND)
          unset(SQLITE_FOUND CACHE)
      else(OMIT_LOAD_EXTENSION)
          set(SQLITE_LOAD_EXTENSION ON CACHE STRING "Support for extensions loading in SQLite")
          if(NOT Sqlite_FIND_QUIETLY)
              message(STATUS "Found support for extensions loading in SQLite.")
          endif(NOT Sqlite_FIND_QUIETLY)
      endif(OMIT_LOAD_EXTENSION)
   endif(SQLITE_LOAD_EXTENSION_REQUIRED)
else(SQLITE_FOUND)
   if(Sqlite_FIND_REQUIRED)
      message(FATAL_ERROR "Required package SQLite NOT found")
   else(Sqlite_FIND_REQUIRED)
      message(STATUS "WARNING: SQLite NOT found")
   endif(Sqlite_FIND_REQUIRED)
endif(SQLITE_FOUND)

if(SQLITE_FOUND)
   set(SQLITE_FOUND 1 CACHE INTERNAL "" FORCE)
else(SQLITE_FOUND)
   unset(SQLITE_FOUND CACHE)
   unset(SQLITE_INCLUDE_DIR CACHE)
   unset(SQLITE_LIBRARIES CACHE)
   unset(SQLITE_VERSION CACHE)
   unset(SQLITE_VERSION_STRING CACHE)
   unset(SQLITE_LOAD_EXTENSION CACHE)
endif(SQLITE_FOUND)

mark_as_advanced(SQLITE_FOUND SQLITE_INCLUDE_DIR SQLITE_LIBRARIES SQLITE_VERSION_STRING SQLITE_VERSION
                 SQLITE_SHELL SQLITE_MIN_VERSION SQLITE_MIN_VERSION_MAJOR SQLITE_MIN_VERSION_MINOR
                 SQLITE_MIN_VERSION_PATCH)
