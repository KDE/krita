# SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Ref: BSD-3-Clause

macro(set_test_sdk_compile_definitions _tgt)
    target_compile_definitions(${_tgt} PUBLIC FILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
    if(WIN32)
        target_compile_definitions(${_tgt} PUBLIC FILES_OUTPUT_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    else()
        target_compile_definitions(${_tgt} PUBLIC FILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")
    endif()
endmacro()
