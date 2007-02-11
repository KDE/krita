
if(KDE4_BUILD_TESTS)

include_directories( ${FLAKE_INCLUDES} )

########### next target ###############

set(TestPosition_test_SRCS TestPosition.cpp )

kde4_automoc(${TestPosition_test_SRCS})

kde4_add_executable(TestPosition RUN_UNINSTALLED ${TestPosition_test_SRCS})

target_link_libraries(TestPosition  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestPosition ${EXECUTABLE_OUTPUT_PATH}/TestPosition)

########### next target ###############

set(TestSelection_test_SRCS TestSelection.cpp )

kde4_automoc(${TestSelection_test_SRCS})

kde4_add_executable(TestSelection RUN_UNINSTALLED ${TestSelection_test_SRCS})

target_link_libraries(TestSelection  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestSelection ${EXECUTABLE_OUTPUT_PATH}/TestSelection)

########### next target ###############

set(TestPathShape_test_SRCS TestPathShape.cpp )

kde4_automoc(${TestPathShape_test_SRCS})

kde4_add_executable(TestPathShape RUN_UNINSTALLED ${TestPathShape_test_SRCS})

target_link_libraries(TestPathShape  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestPathShape ${EXECUTABLE_OUTPUT_PATH}/TestPathShape)

########### next target ###############

set(TestPathTool_test_SRCS TestPathTool.cpp )

kde4_automoc(${TestPathTool_test_SRCS})

kde4_add_executable(TestPathTool RUN_UNINSTALLED ${TestPathTool_test_SRCS})

target_link_libraries(TestPathTool  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestPathTool ${EXECUTABLE_OUTPUT_PATH}/TestPathTool)

########### next target ###############

set(TestControlPointMoveCommand_test_SRCS TestControlPointMoveCommand.cpp )

kde4_automoc(${TestControlPointMoveCommand_test_SRCS})

kde4_add_executable(TestControlPointMoveCommand RUN_UNINSTALLED ${TestControlPointMoveCommand_test_SRCS})

target_link_libraries(TestControlPointMoveCommand  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestControlPointMoveCommand ${EXECUTABLE_OUTPUT_PATH}/TestControlPointMoveCommand)

########### next target ###############

set(TestPointTypeCommand_test_SRCS TestPointTypeCommand.cpp )

kde4_automoc(${TestPointTypeCommand_test_SRCS})

kde4_add_executable(TestPointTypeCommand RUN_UNINSTALLED ${TestPointTypeCommand_test_SRCS})

target_link_libraries(TestPointTypeCommand  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestPointTypeCommand ${EXECUTABLE_OUTPUT_PATH}/TestPointTypeCommand)

########### next target ###############

set(TestPointRemoveCommand_test_SRCS TestPointRemoveCommand.cpp )

kde4_automoc(${TestPointRemoveCommand_test_SRCS})

kde4_add_executable(TestPointRemoveCommand RUN_UNINSTALLED ${TestPointRemoveCommand_test_SRCS})

target_link_libraries(TestPointRemoveCommand  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestPointRemoveCommand ${EXECUTABLE_OUTPUT_PATH}/TestPointRemoveCommand)

########### next target ###############

set(TestRemoveSubpathCommand_test_SRCS TestRemoveSubpathCommand.cpp )

kde4_automoc(${TestRemoveSubpathCommand_test_SRCS})

kde4_add_executable(TestRemoveSubpathCommand RUN_UNINSTALLED ${TestRemoveSubpathCommand_test_SRCS})

target_link_libraries(TestRemoveSubpathCommand  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestRemoveSubpathCommand ${EXECUTABLE_OUTPUT_PATH}/TestRemoveSubpathCommand)

########### next target ###############

set(TestShapeAt_test_SRCS TestShapeAt.cpp )

kde4_automoc(${TestShapeAt_test_SRCS})

kde4_add_executable(TestShapeAt RUN_UNINSTALLED ${TestShapeAt_test_SRCS})

target_link_libraries(TestShapeAt  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestShapeAt ${EXECUTABLE_OUTPUT_PATH}/TestShapeAt)

########### next target ###############

set(TestShapePaint_test_SRCS TestShapePainting.cpp )

kde4_automoc(${TestShapePaint_test_SRCS})

kde4_add_executable(TestShapePainting RUN_UNINSTALLED ${TestShapePaint_test_SRCS})

target_link_libraries(TestShapePainting  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(libs-flake-TestShapePainting ${EXECUTABLE_OUTPUT_PATH}/TestShapePainting)

########### next target ###############

endif(KDE4_BUILD_TESTS)
