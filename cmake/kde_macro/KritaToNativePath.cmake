# SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# krita_to_native_path(<path_list> <output_var>)
#
# Converts path separators of the list <path_list> into the native format
# and stores the result in <output_var>
#
# Usage:
#
# krita_to_native_path("${KRITA_PYTHONPATH}" _krita_pythonpath)

function (krita_to_native_path path_list output_var)
    if(WIN32)
        if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
            foreach (_path ${path_list})
                string(REPLACE "//" "/" _path "${_path}")
                string(REPLACE "/" "\\" _path "${_path}")
                list(APPEND _output_var "${_path}")
            endforeach()

            set (${output_var} ${_output_var} PARENT_SCOPE)
        else()
            cmake_path(CONVERT "${path_list}" TO_NATIVE_PATH_LIST _output_var NORMALIZE)
            set (${output_var} ${_output_var} PARENT_SCOPE)
        endif()
    else()
        set (${output_var} ${path_list} PARENT_SCOPE)
    endif()
endfunction()


# krita_to_native_environment_path_list(<path_list> <output_var>)
#
# Converts the list of paths in <path_list> into the native format
# and then joins them into a string suitable to be assigned to
# PATH (or PATH-like) variable. On Windows the string is joined
# using ';' separator. On other platforms with ':' separator.
#
# To avoid conversion back to a list object, the function adds
# separators in a form of generators: $<SEMICOLON> or $<COLON>
#
# Usage:
#
# krita_to_native_environment_path_list("${KRITA_PYTHONPATH}" _krita_pythonpath)


function (krita_to_native_environment_path_list path_list output_var)
    krita_to_native_path("${path_list}" _output_var)

if(WIN32)
    list(JOIN _output_var $<SEMICOLON> _output_var)
else()
    list(JOIN _output_var $<COLON> _output_var)
endif()

    set (${output_var} ${_output_var} PARENT_SCOPE)
endfunction()
