# Find PyQt6
# ~~~~~~~~~~
# SPDX-FileCopyrightText: 2014 Simon Edwards <simon@simonzone.com>
# SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PyQt6 website: http://www.riverbankcomputing.co.uk/pyqt/index.php
#
# Find the installed version of PyQt6. FindPyQt6 should only be called after
# Python has been found.
#
# This file defines the following variables:
#
# PYQT6_VERSION - The version of PyQt6 found expressed as a 6 digit hex number
#     suitable for comparison as a string
#
# PYQT6_VERSION_STR - The version of PyQt6 as a human readable string.
#
# PYQT6_SIP_DIR - The directory holding the PyQt6 .sip files.

IF(EXISTS PYQT6_VERSION)
  # Already in cache, be silent
  SET(PYQT6_FOUND TRUE)
ELSE(EXISTS PYQT6_VERSION)

  FIND_FILE(_find_pyqt6_py FindPyQt6.py PATHS ${CMAKE_MODULE_PATH})


  if (WIN32)
    # python modules need Qt and C++ libraries be added via explicit
    # calls to os.add_dll_directory(), so we should provide it with
    # correct paths
    get_target_property(LIBQT6CORE_PATH Qt6::Core IMPORTED_LOCATION_RELEASE)
    get_filename_component(LIBQT6CORE_PATH ${LIBQT6CORE_PATH} PATH)
    get_filename_component(MINGW_PATH ${CMAKE_CXX_COMPILER} PATH)

    set(_pyqt6_python_path "${KRITA_PYTHONPATH_V4};${KRITA_PYTHONPATH_V5};$ENV{PYTHONPATH}")

    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E env
      "PYTHONPATH=${_pyqt6_python_path}"
      "PYTHONDLLPATH=${LIBQT6CORE_PATH};${MINGW_PATH};"
      ${Python_EXECUTABLE} ${_find_pyqt6_py}
      OUTPUT_VARIABLE pyqt6_config)
  else (WIN32)
    set(_pyqt6_python_path "${KRITA_PYTHONPATH_V4}:${KRITA_PYTHONPATH_V5}:$ENV{PYTHONPATH}")

    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E env 
      "PYTHONPATH=${_pyqt6_python_path}"
      ${Python_EXECUTABLE} ${_find_pyqt6_py}
      OUTPUT_VARIABLE pyqt6_config)
  endif (WIN32)

  IF(pyqt6_config)
    STRING(REGEX REPLACE "^pyqt_version:([^\n]+).*$" "\\1" PYQT6_VERSION ${pyqt6_config})
    STRING(REGEX REPLACE ".*\npyqt_version_str:([^\n]+).*$" "\\1" PYQT6_VERSION_STR ${pyqt6_config})
    STRING(REGEX REPLACE ".*\npyqt_sip_dir:([^\n]+).*$" "\\1" PYQT6_SIP_DIR ${pyqt6_config})

    SET(PYQT6_FOUND TRUE)
  ENDIF(pyqt6_config)

  IF(PYQT6_FOUND)
    IF(NOT PYQT6_FIND_QUIETLY)
      MESSAGE(STATUS "Found PyQt6 version: ${PYQT6_VERSION_STR}")
    ENDIF(NOT PYQT6_FIND_QUIETLY)
  ELSE(PYQT6_FOUND)
    IF(PYQT6_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PyQt6.")
    ENDIF(PYQT6_FIND_REQUIRED)
  ENDIF(PYQT6_FOUND)

ENDIF(EXISTS PYQT6_VERSION)
