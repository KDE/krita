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
# PYTHON_INCLUDE_PATH, PYTHON_INCLUDE_DIRS - Directory holding the python.h include file.
#
# PYTHON_LIBRARY, PYTHON_LIBRARIES- Location of the Python library.

# SPDX-FileCopyrightText: 2007 Simon Edwards <simon@simonzone.com>
# SPDX-FileCopyrightText: 2012 Luca Beltrame <lbeltrame@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(FindPackageHandleStandardArgs)

if (ENABLE_PYTHON_2)
    find_package(Python 2.7 REQUIRED COMPONENTS Development Interpreter)
else(ENABLE_PYTHON_2)
    if (MINGW)
        find_package(Python 3.8 REQUIRED COMPONENTS Development Interpreter)
    else()
        find_package(Python 3.0 REQUIRED COMPONENTS Interpreter OPTIONAL_COMPONENTS Development)
    endif()
endif(ENABLE_PYTHON_2)

message(STATUS "FindPythonLibrary: ${Python_Interpreter_FOUND}")

if (Python_Interpreter_FOUND)
    set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

    # Set the Python libraries to what we actually found for interpreters
    set(Python_ADDITIONAL_VERSIONS "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
    # These are kept for compatibility
    set(PYTHON_SHORT_VERSION "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
    set(PYTHON_LONG_VERSION ${PYTHON_VERSION_STRING})

    if(Python_Development_FOUND)
        set(PYTHON_INCLUDE_DIRS ${Python_INCLUDE_DIRS})
        set(PYTHON_INCLUDE_PATH ${Python_INCLUDE_DIRS})
        set(PYTHON_LIBRARY ${Python_LIBRARIES})
    endif(Python_Development_FOUND)

    # Auto detect Python site-packages directory
    execute_process(COMMAND ${Python_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(True))"
                    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                   )

    message(STATUS "Python system site-packages directory: ${PYTHON_SITE_PACKAGES_DIR}")

    unset(KRITA_PYTHONPATH_V4 CACHE)
    unset(KRITA_PYTHONPATH_V5 CACHE)
    set(_python_prefix_path ${CMAKE_PREFIX_PATH})
    if (WIN32)
        foreach(__p ${_python_prefix_path})
            set(KRITA_PYTHONPATH_V4 "${__p}/lib/krita-python-libs;${KRITA_PYTHONPATH_V4}")
            set(KRITA_PYTHONPATH_V5 "${__p}/Lib/site-packages;${KRITA_PYTHONPATH_V5}")
        endforeach()
    else()
        foreach(__p ${_python_prefix_path})
            set(KRITA_PYTHONPATH_V4 "${__p}/lib/krita-python-libs:${KRITA_PYTHONPATH_V4}")
            set(KRITA_PYTHONPATH_V5 "${__p}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages:${KRITA_PYTHONPATH_V5}")
        endforeach()
    endif()

    message(STATUS "Krita site-packages directories for SIP v4: ${KRITA_PYTHONPATH_V4}")
    message(STATUS "Krita site-packages directories for SIP v5+: ${KRITA_PYTHONPATH_V5}")
endif()

find_package_handle_standard_args(PythonLibrary DEFAULT_MSG PYTHON_LIBRARY PYTHON_INCLUDE_DIRS PYTHON_INCLUDE_PATH)
