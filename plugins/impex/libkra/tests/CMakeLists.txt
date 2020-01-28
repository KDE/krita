set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()
macro_add_unittest_definitions()

ecm_add_tests(
    kis_kra_loader_test.cpp
    LINK_LIBRARIES kritaui kritalibkra Qt5::Test
    NAME_PREFIX "plugins-impex-")

    
krita_add_broken_unit_test(kis_kra_saver_test.cpp
    TEST_NAME kis_kra_saver_test.cpp
    LINK_LIBRARIES kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-")
    
