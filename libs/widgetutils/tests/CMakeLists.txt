set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories (
    ${CMAKE_SOURCE_DIR}/libs/widgetutils
    ${CMAKE_SOURCE_DIR}/sdk/tests
)

include(ECMAddTests)

ecm_add_tests(
    KisActionsSnapshotTest.cpp
    KoPropertiesTest.cpp
    kis_simple_math_parser_test.cpp # FIXME this test should be in the ui directory
    TestKoProgressUpdater.cpp
    TestSqueezedComboBox.cpp
    NAME_PREFIX "libs-widgetutils-"
    LINK_LIBRARIES kritawidgetutils kritaimage Qt5::Test
)
