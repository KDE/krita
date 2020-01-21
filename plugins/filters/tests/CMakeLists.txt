set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(    )

macro_add_unittest_definitions()
##### Tests that currently fail and should be fixed #####

include(KritaAddBrokenUnitTest)

krita_add_broken_unit_test( kis_all_filter_test.cpp
    TEST_NAME kis_all_filter_test.cpp
    NAME_PREFIX "krita-filters-"
    LINK_LIBRARIES kritaimage Qt5::Test)
    
krita_add_broken_unit_test( kis_crash_filter_test.cpp
    TEST_NAME kis_crash_filter_test.cpp
    NAME_PREFIX "krita-filters-"
    LINK_LIBRARIES kritaimage Qt5::Test)
