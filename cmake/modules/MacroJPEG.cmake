# A macro wrapper to find JPEG library version
#
# Syntax: DETECT_JPEG()
# JPEG_LIB_VERSION is set to version ID depending of libjpeg version detected.
#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# check version of libjpeg so that we can use the appropriate dir
# See bug #227313 for details

function(CompileToCheckVersion LibId Ret)

    set(_jpeglib_version_source "
    #include <stddef.h>
    #include <stdio.h>
    #include <jpeglib.h>
    int main()
    {
    #if (JPEG_LIB_VERSION >= ${LibId})
    #error JPEG_LIB_VERSION >= ${LibId}
    #endif
    }
    ")

    set(_jpeglib_version_source_file ${CMAKE_BINARY_DIR}/CMakeTmp/cmake_jpeglib_version_check.cpp)
    file(WRITE "${_jpeglib_version_source_file}" "${_jpeglib_version_source}")
    set(_jpeglib_version_include_dirs "-DINCLUDE_DIRECTORIES:STRING=${JPEG_INCLUDE_DIR}")

    try_compile(_jpeglib_version_compile_result ${CMAKE_BINARY_DIR} ${_jpeglib_version_source_file}
                CMAKE_FLAGS "${_jpeglib_version_include_dirs}"
                COMPILE_OUTPUT_VARIABLE _jpeglib_version_compile_output_var)

    set(${Ret} ${_jpeglib_version_compile_result} PARENT_SCOPE)

endfunction()

macro(DETECT_JPEG)

    find_package(JPEG REQUIRED)

    if(JPEG_FOUND)

        CompileToCheckVersion(90 _CompileResult)

        if(_CompileResult)

            # Compile sucessfuly. It's not libjpeg 90. We check previous version.

            CompileToCheckVersion(80 _CompileResult)

            if(_CompileResult)

                # Compile sucessfuly. It's not libjpeg 90. We check previous version.

                CompileToCheckVersion(70 _CompileResult)

                if(_CompileResult)

                    # Compile sucessfuly. It's not libjpeg 70.
                    set(JPEG_LIB_VERSION 62)

                else()

                    set(JPEG_LIB_VERSION 70)

                endif()

            else()

                set(JPEG_LIB_VERSION 84)

            endif()

        else()

            set(JPEG_LIB_VERSION 91)

        endif()

        message(STATUS "Libjpeg version: ${JPEG_LIB_VERSION}")

    endif()

endmacro()
