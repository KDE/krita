set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(    )

macro_add_unittest_definitions()

########### next target ###############

ecm_add_tests(
    kis_all_filter_test.cpp
    kis_crash_filter_test.cpp
    NAME_PREFIX "krita-filters-"
    LINK_LIBRARIES kritaimage Qt5::Test)
