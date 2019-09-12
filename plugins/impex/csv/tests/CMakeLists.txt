set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

krita_add_broken_unit_tests (
    kis_csv_test.cpp
    LINK_LIBRARIES kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-")
