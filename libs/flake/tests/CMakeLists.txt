set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${FLAKE_INCLUDES} )

########### next target ###############

set(TestPosition_test_SRCS TestPosition.cpp )
kde4_add_unit_test(TestPosition TESTNAME libs-flake-TestPosition ${TestPosition_test_SRCS})
target_link_libraries(TestPosition  flake Qt5::Test)

########### next target ###############

set(TestSelection_test_SRCS TestSelection.cpp )
kde4_add_unit_test(TestSelection TESTNAME libs-flake-TestSelection ${TestSelection_test_SRCS})
target_link_libraries(TestSelection  flake Qt5::Test)

########### next target ###############

set(TestPathShape_test_SRCS TestPathShape.cpp )
kde4_add_unit_test(TestPathShape TESTNAME libs-flake-TestPathShape ${TestPathShape_test_SRCS})
target_link_libraries(TestPathShape  flake Qt5::Test)

########### next target ###############

set(TestPathTool_test_SRCS TestPathTool.cpp )
kde4_add_unit_test(TestPathTool TESTNAME libs-flake-TestPathTool ${TestPathTool_test_SRCS})
target_link_libraries(TestPathTool  flake Qt5::Test)

########### next target ###############

set(TestControlPointMoveCommand_test_SRCS TestControlPointMoveCommand.cpp )
kde4_add_unit_test(TestControlPointMoveCommand TESTNAME libs-flake-TestControlPointMoveCommand ${TestControlPointMoveCommand_test_SRCS})
target_link_libraries(TestControlPointMoveCommand  flake Qt5::Test)

########### next target ###############

set(TestPointTypeCommand_test_SRCS TestPointTypeCommand.cpp )
kde4_add_unit_test(TestPointTypeCommand TESTNAME libs-flake-TestPointTypeCommand ${TestPointTypeCommand_test_SRCS})
target_link_libraries(TestPointTypeCommand  flake Qt5::Test)

########### next target ###############

set(TestPointRemoveCommand_test_SRCS TestPointRemoveCommand.cpp )
kde4_add_unit_test(TestPointRemoveCommand TESTNAME libs-flake-TestPointRemoveCommand  ${TestPointRemoveCommand_test_SRCS})
target_link_libraries(TestPointRemoveCommand  flake Qt5::Test)

########### next target ###############

set(TestRemoveSubpathCommand_test_SRCS TestRemoveSubpathCommand.cpp )
kde4_add_unit_test(TestRemoveSubpathCommand TESTNAME libs-flake-TestRemoveSubpathCommand ${TestRemoveSubpathCommand_test_SRCS})
target_link_libraries(TestRemoveSubpathCommand  flake Qt5::Test)

########### next target ###############

set(TestShapeAt_test_SRCS TestShapeAt.cpp )
kde4_add_unit_test(TestShapeAt TESTNAME libs-flake-TestShapeAt ${TestShapeAt_test_SRCS})
target_link_libraries(TestShapeAt  flake Qt5::Test)

########### next target ###############

set(TestShapePaint_test_SRCS TestShapePainting.cpp )
kde4_add_unit_test(TestShapePainting TESTNAME libs-flake-TestShapePainting ${TestShapePaint_test_SRCS})
target_link_libraries(TestShapePainting  flake Qt5::Test)

########### next target ###############

set(TestKoShapeFactory_test_SRCS TestKoShapeFactory.cpp )
kde4_add_unit_test(TestKoShapeFactory TESTNAME libs-flake-TestKoShapeFactory ${TestKoShapeFactory_test_SRCS})
target_link_libraries(TestKoShapeFactory  flake Qt5::Test)

########### next target ###############

set(TestKoShapeRegistry_test_SRCS TestKoShapeRegistry.cpp )
kde4_add_unit_test(TestKoShapeRegistry TESTNAME libs-flake-TestKoShapeRegistry  ${TestKoShapeRegistry_test_SRCS})
target_link_libraries(TestKoShapeRegistry  flake Qt5::Test)

########### end ###############

set(TestShapeContainer_test_SRCS TestShapeContainer.cpp )
kde4_add_unit_test(TestShapeContainer TESTNAME libs-flake-TestShapeContainer  ${TestShapeContainer_test_SRCS})
target_link_libraries(TestShapeContainer  flake Qt5::Test)

########### end ###############

set(TestShapeGroupCommand_test_SRCS TestShapeGroupCommand.cpp )
kde4_add_unit_test(TestShapeGroupCommand TESTNAME libs-flake-TestShapeGroupCommand  ${TestShapeGroupCommand_test_SRCS})
target_link_libraries(TestShapeGroupCommand  flake Qt5::Test)

########### end ###############

set(TestShapeReorderCommand_test_SRCS TestShapeReorderCommand.cpp )
kde4_add_unit_test(TestShapeReorderCommand TESTNAME libs-flake-TestShapeReorderCommand  ${TestShapeReorderCommand_test_SRCS})
target_link_libraries(TestShapeReorderCommand  flake Qt5::Test)

########### next target ###############

set(TestPathSegment_test_SRCS TestPathSegment.cpp )
kde4_add_unit_test(TestPathSegment TESTNAME libs-flake-TestPathSegment ${TestPathSegment_test_SRCS})
target_link_libraries(TestPathSegment  flake Qt5::Test)

########### next target ###############

set(TestImageCollection_test_SRCS TestImageCollection.cpp )
kde4_add_unit_test(TestImageCollection TESTNAME libs-flake-TestImageCollection ${TestImageCollection_test_SRCS})
target_link_libraries(TestImageCollection  flake Qt5::Test)

########### next target ###############

set(TestCanvasResourceProvider_test_SRCS TestResourceManager.cpp )
kde4_add_unit_test(TestResourceManager TESTNAME libs-flake-TestResourceManager ${TestCanvasResourceProvider_test_SRCS})
target_link_libraries(TestResourceManager  flake Qt5::Test)

########### end ###############

set(TestShapeBackgroundCommand_test_SRCS TestShapeBackgroundCommand.cpp )
kde4_add_unit_test(TestShapeBackgroundCommand TESTNAME libs-flake-TestShapeBackgroundCommand  ${TestShapeBackgroundCommand_test_SRCS})
target_link_libraries(TestShapeBackgroundCommand  flake Qt5::Test)

########### end ###############

set(TestShapeStrokeCommand_test_SRCS TestShapeStrokeCommand.cpp )
kde4_add_unit_test(TestShapeStrokeCommand TESTNAME libs-flake-TestShapeStrokeCommand  ${TestShapeStrokeCommand_test_SRCS})
target_link_libraries(TestShapeStrokeCommand  flake Qt5::Test)

########### end ###############

set(TestShapeShadowCommand_test_SRCS TestShapeShadowCommand.cpp )
kde4_add_unit_test(TestShapeShadowCommand TESTNAME libs-flake-TestShapeShadowCommand  ${TestShapeShadowCommand_test_SRCS})
target_link_libraries(TestShapeShadowCommand  flake Qt5::Test)

########### end ###############

set(TestSegmentTypeCommand_test_SRCS TestSegmentTypeCommand.cpp )
kde4_add_unit_test(TestSegmentTypeCommand TESTNAME libs-flake-TestSegmentTypeCommand  ${TestSegmentTypeCommand_test_SRCS})
target_link_libraries(TestSegmentTypeCommand  flake Qt5::Test)

########### end ###############

set(TestPointMergeCommand_test_SRCS TestPointMergeCommand.cpp )
kde4_add_unit_test(TestPointMergeCommand TESTNAME libs-flake-TestPointMergeCommand  ${TestPointMergeCommand_test_SRCS})
target_link_libraries(TestPointMergeCommand  flake Qt5::Test)

########### end ###############

set(TestInputDevice_test_SRCS TestInputDevice.cpp )
kde4_add_unit_test(TestInputDevice TESTNAME libs-flake-TestInputDevice  ${TestInputDevice_test_SRCS})
target_link_libraries(TestInputDevice  flake Qt5::Test)

########### end ###############

set(TestSnapStrategy_test_SRCS TestSnapStrategy.cpp )
kde4_add_unit_test(TestSnapStrategy TESTNAME libs-flake-TestSnapStrategy  ${TestSnapStrategy_test_SRCS})
target_link_libraries(TestSnapStrategy  flake Qt5::Test)
