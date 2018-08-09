set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)
include(KritaAddBrokenUnitTest)


macro_add_unittest_definitions()

ecm_add_tests(
    TestPosition.cpp
    TestSelection.cpp
    TestPathTool.cpp
    TestShapeAt.cpp
    TestShapePainting.cpp
    TestKoShapeFactory.cpp
    TestKoShapeRegistry.cpp
    TestShapeContainer.cpp
    TestShapeGroupCommand.cpp
    TestShapeReorderCommand.cpp
    TestImageCollection.cpp
    TestResourceManager.cpp
    TestShapeBackgroundCommand.cpp
    TestShapeStrokeCommand.cpp
    TestShapeShadowCommand.cpp
    TestInputDevice.cpp
    TestSnapStrategy.cpp
    NAME_PREFIX "libs-kritaflake-"
    LINK_LIBRARIES kritaflake Qt5::Test)

ecm_add_test(TestPathShape.cpp
    TEST_NAME libs-kritaflake-TestPathShape
    LINK_LIBRARIES kritaflake Qt5::Test)

ecm_add_test(TestControlPointMoveCommand.cpp
    TEST_NAME libs-kritaflake-TestControlPointMoveCommand
    LINK_LIBRARIES kritaflake Qt5::Test)

ecm_add_test(TestPointTypeCommand.cpp
    TEST_NAME libs-kritaflake-TestPointTypeCommand
    LINK_LIBRARIES kritaflake Qt5::Test)

ecm_add_test(TestPointRemoveCommand.cpp
    TEST_NAME libs-kritaflake-TestPointRemoveCommand
    LINK_LIBRARIES kritaflake Qt5::Test)

ecm_add_test(TestRemoveSubpathCommand.cpp
    TEST_NAME libs-kritaflake-TestRemoveSubpathCommand
    LINK_LIBRARIES kritaflake Qt5::Test)

ecm_add_test(TestPathSegment.cpp
    TEST_NAME libs-kritaflake-TestPathSegment
    LINK_LIBRARIES kritaflake Qt5::Test)

ecm_add_test(TestSegmentTypeCommand.cpp
    TEST_NAME libs-kritaflake-TestSegmentTypeCommand
    LINK_LIBRARIES kritaflake Qt5::Test)

krita_add_broken_unit_test(TestPointMergeCommand.cpp
    TEST_NAME libs-kritaflake-TestPointMergeCommand
    LINK_LIBRARIES kritaflake Qt5::Test)

ecm_add_test(
    TestKoDrag.cpp
    TEST_NAME libs-kritaflake-TestKoDrag
    LINK_LIBRARIES kritaflake Qt5::Test
)

ecm_add_test(
    TestKoMarkerCollection.cpp
    TEST_NAME libs-kritaflake-TestKoMarkerCollection
    LINK_LIBRARIES kritaflake Qt5::Test
)

ecm_add_test(
    TestSvgParser.cpp
    TEST_NAME libs-kritaflake-TestSvgParser
    LINK_LIBRARIES kritaflake Qt5::Test
)

ecm_add_test(
    TestSvgParser.cpp
    TEST_NAME libs-kritaflake-TestSvgParserCloned
    LINK_LIBRARIES kritaflake Qt5::Test
)
set_property(TARGET libs-kritaflake-TestSvgParserCloned
             PROPERTY COMPILE_DEFINITIONS USE_CLONED_SHAPES)

ecm_add_test(
    TestSvgParser.cpp
    TEST_NAME libs-kritaflake-TestSvgParserRoundTrip
    LINK_LIBRARIES kritaflake Qt5::Test
)
set_property(TARGET libs-kritaflake-TestSvgParserRoundTrip
             PROPERTY COMPILE_DEFINITIONS USE_ROUND_TRIP)

krita_add_broken_unit_test(
    TestSvgText.cpp
    TEST_NAME libs-kritaflake-TestSvgText
    LINK_LIBRARIES kritaflake Qt5::Test
)

krita_add_broken_unit_test(
    TestSvgText.cpp
    TEST_NAME libs-kritaflake-TestSvgTextCloned
    LINK_LIBRARIES kritaflake Qt5::Test
)
set_property(TARGET libs-kritaflake-TestSvgTextCloned
             PROPERTY COMPILE_DEFINITIONS USE_CLONED_SHAPES)

krita_add_broken_unit_test(
    TestSvgText.cpp
    TEST_NAME libs-kritaflake-TestSvgTextRoundTrip
    LINK_LIBRARIES kritaflake Qt5::Test
)
set_property(TARGET libs-kritaflake-TestSvgTextRoundTrip
             PROPERTY COMPILE_DEFINITIONS USE_ROUND_TRIP)
