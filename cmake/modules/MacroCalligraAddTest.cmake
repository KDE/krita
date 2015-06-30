#
# Copyright (c) 2012-2015 Jarosław Staniek <staniek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Adds a single test with proper prefix.
# Prefixes help to select subsets of tests to run, e.g. only Krita tests or Flake tests.
#
# Usage: calligra_add_test(<prefix> <test_basename> <libraries_to_link>)
#
# Result: adds a test named "<prefix>-<test_basename>", with executable "<test_basename>"
#         and links it with libraries listed in <libraries_to_link>.
#
# Example: calligra_add_test(kexi GlobalSearchTest keximain kexicore kexiextendedwidgets)
# -- adds a "kexi-GlobalSearchTest" test with executable GlobalSearchTest and links it with
#    libraries keximain, kexicore, kexiextendedwidgets.
#

macro(CALLIGRA_ADD_TEST __test_prefix __test_basename)
    set(_args "")
    list(APPEND _libs ${ARGV})
    list(REMOVE_AT _libs 0 1)

    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
    add_executable(${__test_basename} ${__test_basename}.cpp)
    add_test(${__test_prefix}-${__test_basename} ${__test_basename})
    ecm_mark_as_test(${__test_prefix}-${__test_basename})
    target_link_libraries(${__test_basename} Qt5::Test ${_libs})
    get_property(_COMPILE_FLAGS TARGET ${__test_basename} PROPERTY COMPILE_FLAGS)
    set_property(TARGET ${__test_basename} PROPERTY COMPILE_FLAGS
        ${_COMPILE_FLAGS}\ -DCURRENT_SOURCE_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\""\ -DFILES_OUTPUT_DIR="\\"${CMAKE_CURRENT_BINARY_DIR}/\\"")
#if (BUILD_TEST_COVERAGE)
#    target_link_libraries(${_test_name} gcov)
#endif ()
endmacro(CALLIGRA_ADD_TEST)
