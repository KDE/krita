
if(KDE4_BUILD_TESTS)

include_directories( ${KOPAGEAPP_INCLUDES} )

########### next target ###############

set(TestPAPageInsertCommand_test_SRCS TestPAPageInsertCommand.cpp )

kde4_automoc(${TestPAPageInsertCommand_test_SRCS})

kde4_add_executable(TestPAPageInsertCommand RUN_UNINSTALLED ${TestPAPageInsertCommand_test_SRCS})

target_link_libraries(TestPAPageInsertCommand  ${KDE4_KDECORE_LIBS} kopageapp ${QT_QTTEST_LIBRARY})

add_test(libs-kopageapp-TestPAPageInsertCommand ${EXECUTABLE_OUTPUT_PATH}/TestPAPageInsertCommand)

########### next target ###############

endif(KDE4_BUILD_TESTS)
