include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

macro_add_unittest_definitions()

kis_add_test(
    kis_jpegxl_test.cpp
    TEST_NAME kis_jpegxl_test
    LINK_LIBRARIES kritametadata kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-"
)

macos_test_fixrpath(kis_jpegxl_test)
