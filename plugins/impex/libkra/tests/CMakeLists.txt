set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

ecm_add_test(
    kis_kra_loader_test.cpp
    TEST_NAME plugins-impex-KisKraLoaderTest
    LINK_LIBRARIES kritaimage kritaui kritalibkra Qt5::Test)

ecm_add_test(
    kis_kra_saver_test.cpp
    TEST_NAME plugins-impex-KisKraSaverTest
    LINK_LIBRARIES kritaimage kritaui kritalibkra Qt5::Test)
