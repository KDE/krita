
include(KritaAddBrokenUnitTest)

kis_add_tests(
    TestPosition.cpp
    TestSelection.cpp
    TestPathTool.cpp
    TestShapeAt.cpp
    TestShapePainting.cpp
    TestKoShapeFactory.cpp
    TestShapeContainer.cpp
    TestShapeGroupCommand.cpp
    TestShapeReorderCommand.cpp
    TestResourceManager.cpp
    TestShapeBackgroundCommand.cpp
    TestShapeStrokeCommand.cpp
    TestInputDevice.cpp
    TestSnapStrategy.cpp
    TestPathShape.cpp
    TestControlPointMoveCommand.cpp
    TestPointTypeCommand.cpp
    TestPointRemoveCommand.cpp
    TestRemoveSubpathCommand.cpp
    TestPathSegment.cpp
    TestSegmentTypeCommand.cpp
    TestKoDrag.cpp
    TestKoMarkerCollection.cpp
    TestSvgSavingContext.cpp
    TestXsimdPainting.cpp
    TestFontLibraryResourceUtils.cpp
    TestSvgTextShape.cpp
    TestDependentShapes.cpp

    LINK_LIBRARIES kritaflake kritatestsdk
    NAME_PREFIX "libs-flake-"
    )

kis_add_test(
    TestSvgParser.cpp
    TEST_NAME TestSvgParser
    LINK_LIBRARIES kritaflake kritatestsdk
    NAME_PREFIX "libs-flake-")

kis_add_test(
    TestSvgParser.cpp
    TEST_NAME TestSvgParserCloned
    LINK_LIBRARIES kritaflake kritatestsdk
    NAME_PREFIX "libs-flake-")
target_compile_definitions(TestSvgParserCloned PRIVATE USE_CLONED_SHAPES)

kis_add_test(
    TestSvgParser.cpp
    TEST_NAME TestSvgParserRoundTrip
    LINK_LIBRARIES kritaflake kritatestsdk
    NAME_PREFIX "libs-flake-")
target_compile_definitions(TestSvgParserRoundTrip PRIVATE USE_ROUND_TRIP)

############## broken tests ###############

kis_add_test(TestPointMergeCommand.cpp
    TEST_NAME TestPointMergeCommand
    LINK_LIBRARIES kritaflake kritatestsdk
    NAME_PREFIX "libs-flake-")

krita_add_broken_unit_test( TestSvgText.cpp
    TEST_NAME TestSvgText
    LINK_LIBRARIES kritaflake kritatestsdk
    NAME_PREFIX "libs-flake-")

krita_add_broken_unit_test( TestSvgText.cpp
    TEST_NAME TestSvgTextCloned
    LINK_LIBRARIES kritaflake kritatestsdk
    NAME_PREFIX "libs-flake-")
target_compile_definitions(TestSvgTextCloned PRIVATE USE_CLONED_SHAPES)

krita_add_broken_unit_test( TestSvgText.cpp
    TEST_NAME TestSvgTextRoundTrip
    LINK_LIBRARIES kritaflake kritatestsdk
    NAME_PREFIX "libs-flake-")
target_compile_definitions(TestSvgTextRoundTrip PRIVATE USE_ROUND_TRIP)

if (APPLE)
    set_property(TARGET TestKoMarkerCollection PROPERTY MACOSX_BUNDLE ON)
    set_property(TARGET TestSvgParser PROPERTY MACOSX_BUNDLE ON)
    set_property(TARGET TestSvgParserCloned PROPERTY MACOSX_BUNDLE ON)
    set_property(TARGET TestSvgParserRoundTrip PROPERTY MACOSX_BUNDLE ON)
endif()
