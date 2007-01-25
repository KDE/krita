
if(KDE4_BUILD_TESTS)

include_directories( ${FLAKE_INCLUDES} )

########### next target ###############

set(TestPosition_test_SRCS TestPosition.cpp )

kde4_automoc(${TestPosition_test_SRCS})

kde4_add_executable(TestPosition RUN_UNINSTALLED ${TestPosition_test_SRCS})

target_link_libraries(TestPosition  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(flake-TestPosition ${EXECUTABLE_OUTPUT_PATH}/TestPosition)

########### next target ###############

set(TestSelection_test_SRCS TestSelection.cpp )

kde4_automoc(${TestSelection_test_SRCS})

kde4_add_executable(TestSelection RUN_UNINSTALLED ${TestSelection_test_SRCS})

target_link_libraries(TestSelection  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(flake-TestSelection ${EXECUTABLE_OUTPUT_PATH}/TestSelection)

########### next target ###############

set(TestPathShape_test_SRCS TestPathShape.cpp )

kde4_automoc(${TestPathShape_test_SRCS})

kde4_add_executable(TestPathShape RUN_UNINSTALLED ${TestPathShape_test_SRCS})

target_link_libraries(TestPathShape  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(flake-TestPathShape ${EXECUTABLE_OUTPUT_PATH}/TestPathShape)

########### next target ###############

set(TestPathTool_test_SRCS TestPathTool.cpp )

kde4_automoc(${TestPathTool_test_SRCS})

kde4_add_executable(TestPathTool RUN_UNINSTALLED ${TestPathTool_test_SRCS})

target_link_libraries(TestPathTool  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(flake-TestPathTool ${EXECUTABLE_OUTPUT_PATH}/TestPathTool)

########### next target ###############

set(TestShapeAt_test_SRCS TestShapeAt.cpp )

kde4_automoc(${TestShapeAt_test_SRCS})

kde4_add_executable(TestShapeAt RUN_UNINSTALLED ${TestShapeAt_test_SRCS})

target_link_libraries(TestShapeAt  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

add_test(flake-TestShapeAt ${EXECUTABLE_OUTPUT_PATH}/TestShapeAt)

########### next target ###############

set(TestShapePaint_test_SRCS TestShapePainting.cpp )
kde4_automoc(${TestShapePaint_test_SRCS})
kde4_add_executable(TestShapePainting RUN_UNINSTALLED ${TestShapePaint_test_SRCS})
target_link_libraries(TestShapePainting  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})
add_test(TestShapePainting ${EXECUTABLE_OUTPUT_PATH}/TestShapePainting)

########### next target ###############

endif(KDE4_BUILD_TESTS)
