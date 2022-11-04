include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()


kis_add_test(
    kis_jpeg_test.cpp
    TEST_NAME kis_jpeg_test
    LINK_LIBRARIES kritametadata kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-"
    )

