
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

endif(KDE4_BUILD_TESTS)
