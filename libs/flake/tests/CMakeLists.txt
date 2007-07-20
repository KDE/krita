include_directories( ${FLAKE_INCLUDES} )

########### next target ###############

set(TestPosition_test_SRCS TestPosition.cpp )
kde4_add_unit_test(TestPosition TESTNAME libs-flake-TestPosition ${TestPosition_test_SRCS})
target_link_libraries(TestPosition  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSelection_test_SRCS TestSelection.cpp )
kde4_add_unit_test(TestSelection TESTNAME libs-flake-TestSelection ${TestSelection_test_SRCS})
target_link_libraries(TestSelection  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPathShape_test_SRCS TestPathShape.cpp )
kde4_add_unit_test(TestPathShape TESTNAME libs-flake-TestPathShape ${TestPathShape_test_SRCS})
target_link_libraries(TestPathShape  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPathTool_test_SRCS TestPathTool.cpp )
kde4_add_unit_test(TestPathTool TESTNAME libs-flake-TestPathTool ${TestPathTool_test_SRCS})
target_link_libraries(TestPathTool  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestControlPointMoveCommand_test_SRCS TestControlPointMoveCommand.cpp )
kde4_add_unit_test(TestControlPointMoveCommand TESTNAME libs-flake-TestControlPointMoveCommand ${TestControlPointMoveCommand_test_SRCS})
target_link_libraries(TestControlPointMoveCommand  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPointTypeCommand_test_SRCS TestPointTypeCommand.cpp )
kde4_add_unit_test(TestPointTypeCommand TESTNAME libs-flake-TestPointTypeCommand ${TestPointTypeCommand_test_SRCS})
target_link_libraries(TestPointTypeCommand  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPointRemoveCommand_test_SRCS TestPointRemoveCommand.cpp )
kde4_add_unit_test(TestPointRemoveCommand TESTNAME libs-flake-TestPointRemoveCommand  ${TestPointRemoveCommand_test_SRCS})
target_link_libraries(TestPointRemoveCommand  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestRemoveSubpathCommand_test_SRCS TestRemoveSubpathCommand.cpp )
kde4_add_unit_test(TestRemoveSubpathCommand TESTNAME libs-flake-TestRemoveSubpathCommand ${TestRemoveSubpathCommand_test_SRCS})
target_link_libraries(TestRemoveSubpathCommand  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestShapeAt_test_SRCS TestShapeAt.cpp )
kde4_add_unit_test(TestShapeAt TESTNAME libs-flake-TestShapeAt ${TestShapeAt_test_SRCS})
target_link_libraries(TestShapeAt  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestShapePaint_test_SRCS TestShapePainting.cpp )
kde4_add_unit_test(TestShapePainting TESTNAME libs-flake-TestShapePainting ${TestShapePaint_test_SRCS})
target_link_libraries(TestShapePainting  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoShapeFactory_test_SRCS TestKoShapeFactory.cpp )
kde4_add_unit_test(TestKoShapeFactory TESTNAME libs-flake-TestKoShapeFactory ${TestKoShapeFactory_test_SRCS})
target_link_libraries(TestKoShapeFactory  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoShapeRegistry_test_SRCS TestKoShapeRegistry.cpp )
kde4_add_unit_test(TestKoShapeRegistry TESTNAME libs-flake-TestKoShapeRegistry  ${TestKoShapeRegistry_test_SRCS})
target_link_libraries(TestKoShapeRegistry  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### end ###############

set(TestShapeContainer_test_SRCS TestShapeContainer.cpp )
kde4_add_unit_test(TestShapeContainer TESTNAME libs-flake-TestShapeContainer  ${TestShapeContainer_test_SRCS})
target_link_libraries(TestShapeContainer  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

########### end ###############

