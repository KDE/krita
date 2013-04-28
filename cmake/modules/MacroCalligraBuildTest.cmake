# Copyright (c) 2012-2013 Jarosław Staniek <staniek@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(CALLIGRA_BUILD_TEST __test_name)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
    set(_test_name Test${__test_name})
    kde4_add_unit_test(${_test_name} TESTNAME ${_test_name} ${_test_name}.cpp)
    target_link_libraries(${_test_name} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})
    get_property(_COMPILE_FLAGS TARGET ${_test_name} PROPERTY COMPILE_FLAGS)
    set_property(TARGET ${_test_name} PROPERTY COMPILE_FLAGS ${_COMPILE_FLAGS}\ -DFILES_OUTPUT_DIR="\\"${CMAKE_CURRENT_BINARY_DIR}/\\"")
#if (BUILD_TEST_COVERAGE)
#    target_link_libraries(${_test_name} gcov)
#endif (BUILD_TEST_COVERAGE)
endmacro(CALLIGRA_BUILD_TEST)
