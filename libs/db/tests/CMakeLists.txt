set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DFILES_DATA_DIR=${CMAKE_CURRENT_SOURCE_DIR}/data)
add_definitions(-DFILES_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR})

macro(build_test __test_name)
    set(_test_name Test${__test_name})
    kde4_add_unit_test(${_test_name} TESTNAME ${_test_name} ${_test_name}.cpp)
    target_link_libraries(${_test_name} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${CALLIGRADB_LIBS})
#if (BUILD_TEST_COVERAGE)
#    target_link_libraries(${_test_name} gcov)
#endif (BUILD_TEST_COVERAGE)
endmacro()

build_test(Utils)
