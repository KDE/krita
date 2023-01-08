include_directories(
    ${CMAKE_SOURCE_DIR}/sdk/tests
    ${CMAKE_SOURCE_DIR}/libs/psdutils
)

macro_add_unittest_definitions()

set(PSD_TEST_LIBS kritapsdutils Qt5::Test)

kis_add_tests(
    psd_utils_test.cpp
    compression_test.cpp
    NAME_PREFIX "kritapsdutils-"
    LINK_LIBRARIES ${PSD_TEST_LIBS})
