set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_test(KisPdfTest.cpp
    TEST_NAME KisPdfTest
    LINK_LIBRARIES kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-")
