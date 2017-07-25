# Check whether the found python is the same as ext_python
# HACK: Find pythonxx.dll and compare equality. Probably not the best idea...
# TODO: Check the python version

set(_check_python_dll "python36.dll")

if(NOT ${PYTHONLIBS_VERSION_STRING} VERSION_EQUAL "3.6.2")
    message(FATAL_ERROR "Windows build with Python requires Python 3.6.2, found version ${PYTHONLIBS_VERSION_STRING} instead.")
else()
    if(EXISTS "${CMAKE_INSTALL_PREFIX}/python/${_check_python_dll}")
        message(STATUS "python36.dll is found in \"${CMAKE_INSTALL_PREFIX}/python/\".")
        file(SHA1 "${CMAKE_INSTALL_PREFIX}/python/${_check_python_dll}" _ext_python_dll_sha1)
        get_filename_component(_found_python_dir ${PYTHON_EXECUTABLE} DIRECTORY)
        file(SHA1 "${_found_python_dir}/${_check_python_dll}" _found_python_dll_sha1)
        if(NOT ${_ext_python_dll_sha1} STREQUAL ${_found_python_dll_sha1})
            message(FATAL_ERROR "The found ${_check_python_dll} is not the same as the ${_check_python_dll} from ext_python.")
        endif()
    else()
        message(FATAL_ERROR "${_check_python_dll} is NOT found in \"${CMAKE_INSTALL_PREFIX}/python/\".")
    endif()
endif()

unset(_check_python_dll)
