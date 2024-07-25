# SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
# SPDX-License-Identifier: BSD-3-Clause

include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_xsimd QUIET xsimd)

find_package(xsimd QUIET NO_MODULE
    HINTS ${PC_xsimd_CONFIG_DIR} /usr/lib/cmake/xsimd /usr/local/lib/cmake/xsimd
)

if(xsimd_FOUND)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/xsimd")
    include(xsimdMacros)
    xsimd_set_preferred_compiler_flags()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(xsimd CONFIG_MODE REQUIRED_VARS xsimd_IS_CONFIGURATION_VALID)
