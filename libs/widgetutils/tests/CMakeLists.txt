set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories (
    ${CMAKE_SOURCE_DIR}/libs/widgetutils
    ${CMAKE_SOURCE_DIR}/sdk/tests
)

include(ECMAddTests)

ecm_add_tests(
    KisActionsSnapshotTest.cpp
    KoPropertiesTest.cpp
    NAME_PREFIX "libs-widgetutils-"
    LINK_LIBRARIES kritawidgetutils Qt5::Test
)

# FIXME this test should be in the ui directory
ecm_add_test(
    kis_simple_math_parser_test.cpp
    TEST_NAME libs-widgetutils-KisSimpleMathParserTest
    LINK_LIBRARIES kritaui Qt5::Test)

ecm_add_test(
    TestKoProgressUpdater.cpp
    TEST_NAME libs-widgetutils-TestKoProgressUpdater
    LINK_LIBRARIES kritaui Qt5::Test)
