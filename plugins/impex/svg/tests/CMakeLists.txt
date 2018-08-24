set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include(ECMAddTests)
macro_add_unittest_definitions()

ecm_add_test(
    kis_svg_test.cpp
    TEST_NAME KisSvgTest
    LINK_LIBRARIES kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-"
)
