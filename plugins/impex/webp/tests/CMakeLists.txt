include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

macro_add_unittest_definitions()

kis_add_test(
    kis_webp_test.cpp
    TEST_NAME kis_webp_test
    LINK_LIBRARIES kritametadata kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-"
)
