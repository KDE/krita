set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_test(kis_png_test.cpp
    TEST_NAME krita-plugin-format-png_test
    LINK_LIBRARIES kritaui Qt5::Test)
