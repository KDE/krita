# Find Python
# ~~~~~~~~~~~
# Find the Python interpreter and related Python directories.
#
# This file defines the following variables:
#
# PYTHON_EXECUTABLE - The path and filename of the Python interpreter.
#
# PYTHON_SHORT_VERSION - The version of the Python interpreter found,
#     excluding the patch version number. (e.g. 2.5 and not 2.5.1))
#
# PYTHON_LONG_VERSION - The version of the Python interpreter found as a human
#     readable string.
#
# PYTHON_SITE_PACKAGES_DIR - Location of the Python site-packages directory.
#
# PYTHON_INCLUDE_PATH - Directory holding the python.h include file.
#
# PYTHON_LIBRARY, PYTHON_LIBRARIES- Location of the Python library.

# SPDX-FileCopyrightText: 2007 Simon Edwards <simon@simonzone.com>
# SPDX-FileCopyrightText: 2012 Luca Beltrame <lbeltrame@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(FindPackageHandleStandardArgs)

if (ENABLE_PYTHON_2)
    find_package(PythonInterp 2.7 REQUIRED)
else(ENABLE_PYTHON_2)
    if (MINGW)
        find_package(PythonInterp 3.8 REQUIRED)
    else()
        find_package(PythonInterp 3.0 REQUIRED)
    endif()
endif(ENABLE_PYTHON_2)

if (PYTHONINTERP_FOUND)

    # Set the Python libraries to what we actually found for interpreters
    set(Python_ADDITIONAL_VERSIONS "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
    # These are kept for compatibility
    set(PYTHON_SHORT_VERSION "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
    set(PYTHON_LONG_VERSION ${PYTHON_VERSION_STRING})

    find_package(PythonLibs QUIET)

    if(PYTHONLIBS_FOUND)
        set(PYTHON_LIBRARY ${PYTHON_LIBRARIES})
    endif(PYTHONLIBS_FOUND)

    # Auto detect Python site-packages directory
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(True))"
                    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                   )

    message(STATUS "Python system site-packages directory: ${PYTHON_SITE_PACKAGES_DIR}")

endif(PYTHONINTERP_FOUND)

find_package_handle_standard_args(PythonLibrary DEFAULT_MSG PYTHON_LIBRARY)
