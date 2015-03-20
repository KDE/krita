set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${FLAKE_INCLUDES} )

########### next target ###############

set(TestPosition_test_SRCS TestPosition.cpp )
add_executable(TestPositionNAME libs-flake-TestPosition ${TestPosition_test_SRCS})
add_test(TestPosition TestPosition)
ecm_mark_as_test(TestPosition)
target_link_libraries(TestPosition   flake Qt5::Test)

########### next target ###############

set(TestSelection_test_SRCS TestSelection.cpp )
add_executable(TestSelectionNAME libs-flake-TestSelection ${TestSelection_test_SRCS})
add_test(TestSelection TestSelection)
ecm_mark_as_test(TestSelection)
target_link_libraries(TestSelection   flake Qt5::Test)

########### next target ###############

set(TestPathShape_test_SRCS TestPathShape.cpp )
add_executable(TestPathShapeNAME libs-flake-TestPathShape ${TestPathShape_test_SRCS})
add_test(TestPathShape TestPathShape)
ecm_mark_as_test(TestPathShape)
target_link_libraries(TestPathShape   flake Qt5::Test)

########### next target ###############

set(TestPathTool_test_SRCS TestPathTool.cpp )
add_executable(TestPathToolNAME libs-flake-TestPathTool ${TestPathTool_test_SRCS})
add_test(TestPathTool TestPathTool)
ecm_mark_as_test(TestPathTool)
target_link_libraries(TestPathTool   flake Qt5::Test)

########### next target ###############

set(TestControlPointMoveCommand_test_SRCS TestControlPointMoveCommand.cpp )
add_executable(TestControlPointMoveCommandNAME libs-flake-TestControlPointMoveCommand ${TestControlPointMoveCommand_test_SRCS})
add_test(TestControlPointMoveCommand TestControlPointMoveCommand)
ecm_mark_as_test(TestControlPointMoveCommand)
target_link_libraries(TestControlPointMoveCommand   flake Qt5::Test)

########### next target ###############

set(TestPointTypeCommand_test_SRCS TestPointTypeCommand.cpp )
add_executable(TestPointTypeCommandNAME libs-flake-TestPointTypeCommand ${TestPointTypeCommand_test_SRCS})
add_test(TestPointTypeCommand TestPointTypeCommand)
ecm_mark_as_test(TestPointTypeCommand)
target_link_libraries(TestPointTypeCommand   flake Qt5::Test)

########### next target ###############

set(TestPointRemoveCommand_test_SRCS TestPointRemoveCommand.cpp )
add_executable(TestPointRemoveCommandNAME libs-flake-TestPointRemoveCommand  ${TestPointRemoveCommand_test_SRCS})
add_test(TestPointRemoveCommand TestPointRemoveCommand)
ecm_mark_as_test(TestPointRemoveCommand)
target_link_libraries(TestPointRemoveCommand   flake Qt5::Test)

########### next target ###############

set(TestRemoveSubpathCommand_test_SRCS TestRemoveSubpathCommand.cpp )
add_executable(TestRemoveSubpathCommandNAME libs-flake-TestRemoveSubpathCommand ${TestRemoveSubpathCommand_test_SRCS})
add_test(TestRemoveSubpathCommand TestRemoveSubpathCommand)
ecm_mark_as_test(TestRemoveSubpathCommand)
target_link_libraries(TestRemoveSubpathCommand   flake Qt5::Test)

########### next target ###############

set(TestShapeAt_test_SRCS TestShapeAt.cpp )
add_executable(TestShapeAtNAME libs-flake-TestShapeAt ${TestShapeAt_test_SRCS})
add_test(TestShapeAt TestShapeAt)
ecm_mark_as_test(TestShapeAt)
target_link_libraries(TestShapeAt   flake Qt5::Test)

########### next target ###############

set(TestShapePaint_test_SRCS TestShapePainting.cpp )
add_executable(TestShapePaintingNAME libs-flake-TestShapePainting ${TestShapePaint_test_SRCS})
add_test(TestShapePainting TestShapePainting)
ecm_mark_as_test(TestShapePainting)
target_link_libraries(TestShapePainting   flake Qt5::Test)

########### next target ###############

set(TestKoShapeFactory_test_SRCS TestKoShapeFactory.cpp )
add_executable(TestKoShapeFactoryNAME libs-flake-TestKoShapeFactory ${TestKoShapeFactory_test_SRCS})
add_test(TestKoShapeFactory TestKoShapeFactory)
ecm_mark_as_test(TestKoShapeFactory)
target_link_libraries(TestKoShapeFactory   flake Qt5::Test)

########### next target ###############

set(TestKoShapeRegistry_test_SRCS TestKoShapeRegistry.cpp )
add_executable(TestKoShapeRegistryNAME libs-flake-TestKoShapeRegistry  ${TestKoShapeRegistry_test_SRCS})
add_test(TestKoShapeRegistry TestKoShapeRegistry)
ecm_mark_as_test(TestKoShapeRegistry)
target_link_libraries(TestKoShapeRegistry   flake Qt5::Test)

########### end ###############

set(TestShapeContainer_test_SRCS TestShapeContainer.cpp )
add_executable(TestShapeContainerNAME libs-flake-TestShapeContainer  ${TestShapeContainer_test_SRCS})
add_test(TestShapeContainer TestShapeContainer)
ecm_mark_as_test(TestShapeContainer)
target_link_libraries(TestShapeContainer   flake Qt5::Test)

########### end ###############

set(TestShapeGroupCommand_test_SRCS TestShapeGroupCommand.cpp )
add_executable(TestShapeGroupCommandNAME libs-flake-TestShapeGroupCommand  ${TestShapeGroupCommand_test_SRCS})
add_test(TestShapeGroupCommand TestShapeGroupCommand)
ecm_mark_as_test(TestShapeGroupCommand)
target_link_libraries(TestShapeGroupCommand   flake Qt5::Test)

########### end ###############

set(TestShapeReorderCommand_test_SRCS TestShapeReorderCommand.cpp )
add_executable(TestShapeReorderCommandNAME libs-flake-TestShapeReorderCommand  ${TestShapeReorderCommand_test_SRCS})
add_test(TestShapeReorderCommand TestShapeReorderCommand)
ecm_mark_as_test(TestShapeReorderCommand)
target_link_libraries(TestShapeReorderCommand   flake Qt5::Test)

########### next target ###############

set(TestPathSegment_test_SRCS TestPathSegment.cpp )
add_executable(TestPathSegmentNAME libs-flake-TestPathSegment ${TestPathSegment_test_SRCS})
add_test(TestPathSegment TestPathSegment)
ecm_mark_as_test(TestPathSegment)
target_link_libraries(TestPathSegment   flake Qt5::Test)

########### next target ###############

set(TestImageCollection_test_SRCS TestImageCollection.cpp )
add_executable(TestImageCollectionNAME libs-flake-TestImageCollection ${TestImageCollection_test_SRCS})
add_test(TestImageCollection TestImageCollection)
ecm_mark_as_test(TestImageCollection)
target_link_libraries(TestImageCollection   flake Qt5::Test)

########### next target ###############

set(TestCanvasResourceProvider_test_SRCS TestResourceManager.cpp )
add_executable(TestResourceManagerNAME libs-flake-TestResourceManager ${TestCanvasResourceProvider_test_SRCS})
add_test(TestResourceManager TestResourceManager)
ecm_mark_as_test(TestResourceManager)
target_link_libraries(TestResourceManager   flake Qt5::Test)

########### end ###############

set(TestShapeBackgroundCommand_test_SRCS TestShapeBackgroundCommand.cpp )
add_executable(TestShapeBackgroundCommandNAME libs-flake-TestShapeBackgroundCommand  ${TestShapeBackgroundCommand_test_SRCS})
add_test(TestShapeBackgroundCommand TestShapeBackgroundCommand)
ecm_mark_as_test(TestShapeBackgroundCommand)
target_link_libraries(TestShapeBackgroundCommand   flake Qt5::Test)

########### end ###############

set(TestShapeStrokeCommand_test_SRCS TestShapeStrokeCommand.cpp )
add_executable(TestShapeStrokeCommandNAME libs-flake-TestShapeStrokeCommand  ${TestShapeStrokeCommand_test_SRCS})
add_test(TestShapeStrokeCommand TestShapeStrokeCommand)
ecm_mark_as_test(TestShapeStrokeCommand)
target_link_libraries(TestShapeStrokeCommand  flake Qt5::Test)

########### end ###############

set(TestShapeShadowCommand_test_SRCS TestShapeShadowCommand.cpp )
add_executable(TestShapeShadowCommandNAME libs-flake-TestShapeShadowCommand  ${TestShapeShadowCommand_test_SRCS})
add_test(TestShapeShadowCommand TestShapeShadowCommand)
ecm_mark_as_test(TestShapeShadowCommand)
target_link_libraries(TestShapeShadowCommand  flake Qt5::Test)

########### end ###############

set(TestSegmentTypeCommand_test_SRCS TestSegmentTypeCommand.cpp )
add_executable(TestSegmentTypeCommandNAME libs-flake-TestSegmentTypeCommand  ${TestSegmentTypeCommand_test_SRCS})
add_test(TestSegmentTypeCommand TestSegmentTypeCommand)
ecm_mark_as_test(TestSegmentTypeCommand)
target_link_libraries(TestSegmentTypeCommand  flake Qt5::Test)

########### end ###############

set(TestPointMergeCommand_test_SRCS TestPointMergeCommand.cpp )
add_executable(TestPointMergeCommandNAME libs-flake-TestPointMergeCommand  ${TestPointMergeCommand_test_SRCS})
add_test(TestPointMergeCommand TestPointMergeCommand)
ecm_mark_as_test(TestPointMergeCommand)
target_link_libraries(TestPointMergeCommand  flake Qt5::Test)

########### end ###############

set(TestInputDevice_test_SRCS TestInputDevice.cpp )
add_executable(TestInputDeviceNAME libs-flake-TestInputDevice  ${TestInputDevice_test_SRCS})
add_test(TestInputDevice TestInputDevice)
ecm_mark_as_test(TestInputDevice)
target_link_libraries(TestInputDevice  flake Qt5::Test)

########### end ###############

set(TestSnapStrategy_test_SRCS TestSnapStrategy.cpp )
add_executable(TestSnapStrategyNAME libs-flake-TestSnapStrategy  ${TestSnapStrategy_test_SRCS})
add_test(TestSnapStrategy TestSnapStrategy)
ecm_mark_as_test(TestSnapStrategy)
target_link_libraries(TestSnapStrategy  flake Qt5::Test)
