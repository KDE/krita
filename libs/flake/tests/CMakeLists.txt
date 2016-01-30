set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(TestPosition_test_SRCS TestPosition.cpp )
kde4_add_unit_test(TestPosition TESTNAME libs-kritaflake-TestPosition ${TestPosition_test_SRCS})
target_link_libraries(TestPosition  kritaflake Qt5::Test)

########### next target ###############

set(TestSelection_test_SRCS TestSelection.cpp )
kde4_add_unit_test(TestSelection TESTNAME libs-kritaflake-TestSelection ${TestSelection_test_SRCS})
target_link_libraries(TestSelection  kritaflake Qt5::Test)

########### next target ###############

set(TestPathShape_test_SRCS TestPathShape.cpp )
kde4_add_unit_test(TestPathShape TESTNAME libs-kritaflake-TestPathShape ${TestPathShape_test_SRCS})
target_link_libraries(TestPathShape  kritaflake Qt5::Test)

########### next target ###############

set(TestPathTool_test_SRCS TestPathTool.cpp )
kde4_add_unit_test(TestPathTool TESTNAME libs-kritaflake-TestPathTool ${TestPathTool_test_SRCS})
target_link_libraries(TestPathTool  kritaflake Qt5::Test)

########### next target ###############

set(TestControlPointMoveCommand_test_SRCS TestControlPointMoveCommand.cpp )
kde4_add_unit_test(TestControlPointMoveCommand TESTNAME libs-kritaflake-TestControlPointMoveCommand ${TestControlPointMoveCommand_test_SRCS})
target_link_libraries(TestControlPointMoveCommand  kritaflake Qt5::Test)

########### next target ###############

set(TestPointTypeCommand_test_SRCS TestPointTypeCommand.cpp )
kde4_add_unit_test(TestPointTypeCommand TESTNAME libs-kritaflake-TestPointTypeCommand ${TestPointTypeCommand_test_SRCS})
target_link_libraries(TestPointTypeCommand  kritaflake Qt5::Test)

########### next target ###############

set(TestPointRemoveCommand_test_SRCS TestPointRemoveCommand.cpp )
kde4_add_unit_test(TestPointRemoveCommand TESTNAME libs-kritaflake-TestPointRemoveCommand  ${TestPointRemoveCommand_test_SRCS})
target_link_libraries(TestPointRemoveCommand  kritaflake Qt5::Test)

########### next target ###############

set(TestRemoveSubpathCommand_test_SRCS TestRemoveSubpathCommand.cpp )
kde4_add_unit_test(TestRemoveSubpathCommand TESTNAME libs-kritaflake-TestRemoveSubpathCommand ${TestRemoveSubpathCommand_test_SRCS})
target_link_libraries(TestRemoveSubpathCommand  kritaflake Qt5::Test)

########### next target ###############

set(TestShapeAt_test_SRCS TestShapeAt.cpp )
kde4_add_unit_test(TestShapeAt TESTNAME libs-kritaflake-TestShapeAt ${TestShapeAt_test_SRCS})
target_link_libraries(TestShapeAt  kritaflake Qt5::Test)

########### next target ###############

set(TestShapePaint_test_SRCS TestShapePainting.cpp )
kde4_add_unit_test(TestShapePainting TESTNAME libs-kritaflake-TestShapePainting ${TestShapePaint_test_SRCS})
target_link_libraries(TestShapePainting  kritaflake Qt5::Test)

########### next target ###############

set(TestKoShapeFactory_test_SRCS TestKoShapeFactory.cpp )
kde4_add_unit_test(TestKoShapeFactory TESTNAME libs-kritaflake-TestKoShapeFactory ${TestKoShapeFactory_test_SRCS})
target_link_libraries(TestKoShapeFactory  kritaflake Qt5::Test)

########### next target ###############

set(TestKoShapeRegistry_test_SRCS TestKoShapeRegistry.cpp )
kde4_add_unit_test(TestKoShapeRegistry TESTNAME libs-kritaflake-TestKoShapeRegistry  ${TestKoShapeRegistry_test_SRCS})
target_link_libraries(TestKoShapeRegistry  kritaflake Qt5::Test)

########### end ###############

set(TestShapeContainer_test_SRCS TestShapeContainer.cpp )
kde4_add_unit_test(TestShapeContainer TESTNAME libs-kritaflake-TestShapeContainer  ${TestShapeContainer_test_SRCS})
target_link_libraries(TestShapeContainer  kritaflake Qt5::Test)

########### end ###############

set(TestShapeGroupCommand_test_SRCS TestShapeGroupCommand.cpp )
kde4_add_unit_test(TestShapeGroupCommand TESTNAME libs-kritaflake-TestShapeGroupCommand  ${TestShapeGroupCommand_test_SRCS})
target_link_libraries(TestShapeGroupCommand  kritaflake Qt5::Test)

########### end ###############

set(TestShapeReorderCommand_test_SRCS TestShapeReorderCommand.cpp )
kde4_add_unit_test(TestShapeReorderCommand TESTNAME libs-kritaflake-TestShapeReorderCommand  ${TestShapeReorderCommand_test_SRCS})
target_link_libraries(TestShapeReorderCommand  kritaflake Qt5::Test)

########### next target ###############

set(TestPathSegment_test_SRCS TestPathSegment.cpp )
kde4_add_unit_test(TestPathSegment TESTNAME libs-kritaflake-TestPathSegment ${TestPathSegment_test_SRCS})
target_link_libraries(TestPathSegment  kritaflake Qt5::Test)

########### next target ###############

set(TestImageCollection_test_SRCS TestImageCollection.cpp )
kde4_add_unit_test(TestImageCollection TESTNAME libs-kritaflake-TestImageCollection ${TestImageCollection_test_SRCS})
target_link_libraries(TestImageCollection  kritaflake Qt5::Test)

########### next target ###############

set(TestCanvasResourceProvider_test_SRCS TestResourceManager.cpp )
kde4_add_unit_test(TestResourceManager TESTNAME libs-kritaflake-TestResourceManager ${TestCanvasResourceProvider_test_SRCS})
target_link_libraries(TestResourceManager  kritaflake Qt5::Test)

########### end ###############

set(TestShapeBackgroundCommand_test_SRCS TestShapeBackgroundCommand.cpp )
kde4_add_unit_test(TestShapeBackgroundCommand TESTNAME libs-kritaflake-TestShapeBackgroundCommand  ${TestShapeBackgroundCommand_test_SRCS})
target_link_libraries(TestShapeBackgroundCommand  kritaflake Qt5::Test)

########### end ###############

set(TestShapeStrokeCommand_test_SRCS TestShapeStrokeCommand.cpp )
kde4_add_unit_test(TestShapeStrokeCommand TESTNAME libs-kritaflake-TestShapeStrokeCommand  ${TestShapeStrokeCommand_test_SRCS})
target_link_libraries(TestShapeStrokeCommand  kritaflake Qt5::Test)

########### end ###############

set(TestShapeShadowCommand_test_SRCS TestShapeShadowCommand.cpp )
kde4_add_unit_test(TestShapeShadowCommand TESTNAME libs-kritaflake-TestShapeShadowCommand  ${TestShapeShadowCommand_test_SRCS})
target_link_libraries(TestShapeShadowCommand  kritaflake Qt5::Test)

########### end ###############

set(TestSegmentTypeCommand_test_SRCS TestSegmentTypeCommand.cpp )
kde4_add_unit_test(TestSegmentTypeCommand TESTNAME libs-kritaflake-TestSegmentTypeCommand  ${TestSegmentTypeCommand_test_SRCS})
target_link_libraries(TestSegmentTypeCommand  kritaflake Qt5::Test)

########### end ###############

set(TestPointMergeCommand_test_SRCS TestPointMergeCommand.cpp )
kde4_add_unit_test(TestPointMergeCommand TESTNAME libs-kritaflake-TestPointMergeCommand  ${TestPointMergeCommand_test_SRCS})
target_link_libraries(TestPointMergeCommand  kritaflake Qt5::Test)

########### end ###############

set(TestInputDevice_test_SRCS TestInputDevice.cpp )
kde4_add_unit_test(TestInputDevice TESTNAME libs-kritaflake-TestInputDevice  ${TestInputDevice_test_SRCS})
target_link_libraries(TestInputDevice  kritaflake Qt5::Test)

########### end ###############

set(TestSnapStrategy_test_SRCS TestSnapStrategy.cpp )
kde4_add_unit_test(TestSnapStrategy TESTNAME libs-kritaflake-TestSnapStrategy  ${TestSnapStrategy_test_SRCS})
target_link_libraries(TestSnapStrategy  kritaflake Qt5::Test)
