qt4_wrap_cpp(UNIT_TESTS_HEADERS_MOC version_checker_test.h)

add_executable(
    unit_tests
    version_checker_test.cpp
    ${UNIT_TESTS_HEADERS_MOC}
  )

target_link_libraries(
    unit_tests
    ${QT_QTTEST_LIBRARY_RELEASE}
    ${QT_QTCORE_LIBRARY_RELEASE}
  )
