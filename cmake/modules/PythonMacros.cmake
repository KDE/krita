# Python macros
# ~~~~~~~~~~~~~
# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Copyright (c) 2012, Luca Beltrame <lbeltrame@kde.org>
# Copyright (c) 2012, Rolf Eike Beer <eike@sf-mail.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# This file defines the following macros:
#
# PYTHON_INSTALL (SOURCE_FILE DESTINATION_DIR)
#     Install the SOURCE_FILE, which is a Python .py file, into the
#     destination directory during install. The file will be byte compiled
#     and both the .py file and .pyc file will be installed.

set(PYTHON_MACROS_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

macro(PYTHON_INSTALL SOURCE_FILE DESTINATION_DIR)

  find_file(_python_compile_py PythonCompile.py PATHS ${CMAKE_MODULE_PATH})

  # Install the source file.
  install(FILES ${SOURCE_FILE} DESTINATION ${DESTINATION_DIR})

  # Byte compile and install the .pyc file, unless explicitly prevented by env..
  if("$ENV{PYTHONDONTWRITEBYTECODE}" STREQUAL "")
    get_filename_component(_absfilename ${SOURCE_FILE} ABSOLUTE)
    get_filename_component(_filename ${SOURCE_FILE} NAME)
    get_filename_component(_filenamebase ${SOURCE_FILE} NAME_WE)
    get_filename_component(_basepath ${SOURCE_FILE} PATH)

    if(WIN32)
      # remove drive letter
      string(REGEX REPLACE "^[a-zA-Z]:/" "/" _basepath "${_basepath}")
    endif(WIN32)

    set(_bin_py ${CMAKE_CURRENT_BINARY_DIR}/${_basepath}/${_filename})

    # Python 3.2 changed the pyc file location
    if(PYTHON_VERSION_STRING VERSION_GREATER 3.1)
      # To get the right version for suffix
      set(_bin_pyc "${CMAKE_CURRENT_BINARY_DIR}/${_basepath}/__pycache__/${_filenamebase}.cpython-${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.pyc")
      set(_py_install_dir "${DESTINATION_DIR}/__pycache__/")
    else()
      set(_bin_pyc "${CMAKE_CURRENT_BINARY_DIR}/${_basepath}/${_filenamebase}.pyc")
      set(_py_install_dir "${DESTINATION_DIR}")
    endif()

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_basepath})

    # Setting because it will be displayed later, in compile_python_files
    set(_message "Byte-compiling ${_bin_py} to ${_bin_pyc}")

    string(REPLACE "/" "_" _rule_name "${_basepath}/${_bin_pyc}")
    add_custom_target("${_rule_name}" ALL)

    get_filename_component(_abs_bin_py ${_bin_py} ABSOLUTE)
    if(_abs_bin_py STREQUAL _absfilename)    # Don't copy the file onto itself.
      add_custom_command(
        TARGET "${_rule_name}"
        COMMAND "${CMAKE_COMMAND}" -E echo "${_message}"
        COMMAND "${PYTHON_EXECUTABLE}" "${_python_compile_py}" "${_bin_py}"
        DEPENDS "${_absfilename}"
      )
    else()
      add_custom_command(
        TARGET "${_rule_name}"
        COMMAND "${CMAKE_COMMAND}" -E echo "${_message}"
        COMMAND "${CMAKE_COMMAND}" -E copy "${_absfilename}" "${_bin_py}"
        COMMAND "${PYTHON_EXECUTABLE}" "${_python_compile_py}" "${_bin_py}"
        DEPENDS "${_absfilename}"
      )
    endif()

    install(FILES ${_bin_pyc} DESTINATION "${_py_install_dir}")
    unset(_py_install_dir)
    unset(_message)

  endif("$ENV{PYTHONDONTWRITEBYTECODE}" STREQUAL "")

endmacro(PYTHON_INSTALL)
