
if(KDE4_BUILD_TESTS)

include_directories( ${KOPAGEAPP_INCLUDES} )

########### next target ###############

set(TestPAPageInsertCommand_test_SRCS TestPAPageInsertCommand.cpp )

kde4_automoc(${TestPAPageInsertCommand_test_SRCS})

kde4_add_executable(TestPAPageInsertCommand RUN_UNINSTALLED ${TestPAPageInsertCommand_test_SRCS})

target_link_libraries(TestPAPageInsertCommand  ${KDE4_KDECORE_LIBS} kopageapp ${QT_QTTEST_LIBRARY})

add_test(libs-kopageapp-TestPAPageInsertCommand ${EXECUTABLE_OUTPUT_PATH}/TestPAPageInsertCommand)

########### next target ###############

set(TestPAPageDeleteCommand_test_SRCS TestPAPageDeleteCommand.cpp )

kde4_automoc(${TestPAPageDeleteCommand_test_SRCS})

kde4_add_executable(TestPAPageDeleteCommand RUN_UNINSTALLED ${TestPAPageDeleteCommand_test_SRCS})

target_link_libraries(TestPAPageDeleteCommand  ${KDE4_KDECORE_LIBS} kopageapp ${QT_QTTEST_LIBRARY})

add_test(libs-kopageapp-TestPAPageDeleteCommand ${EXECUTABLE_OUTPUT_PATH}/TestPAPageDeleteCommand)

########### next target ###############

set(TestPAPageMoveCommand_test_SRCS TestPAPageMoveCommand.cpp )

kde4_automoc(${TestPAPageMoveCommand_test_SRCS})

kde4_add_executable(TestPAPageMoveCommand RUN_UNINSTALLED ${TestPAPageMoveCommand_test_SRCS})

target_link_libraries(TestPAPageMoveCommand  ${KDE4_KDECORE_LIBS} kopageapp ${QT_QTTEST_LIBRARY})

add_test(libs-kopageapp-TestPAPageMoveCommand ${EXECUTABLE_OUTPUT_PATH}/TestPAPageMoveCommand)

########### next target ###############

endif(KDE4_BUILD_TESTS)
