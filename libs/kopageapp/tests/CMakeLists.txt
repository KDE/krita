set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOPAGEAPP_INCLUDES} )

########### next target ###############

set(TestPAPageInsertCommand_test_SRCS TestPAPageInsertCommand.cpp )

kde4_add_unit_test(TestPAPageInsertCommand TESTNAME libs-kopageapp-TestPAPageInsertCommand ${TestPAPageInsertCommand_test_SRCS})

target_link_libraries(TestPAPageInsertCommand   kopageapp ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPAPageDeleteCommand_test_SRCS TestPAPageDeleteCommand.cpp )

kde4_add_unit_test(TestPAPageDeleteCommand TESTNAME libs-kopageapp-TestPAPageDeleteCommand ${TestPAPageDeleteCommand_test_SRCS})

target_link_libraries(TestPAPageDeleteCommand   kopageapp ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPAPageMoveCommand_test_SRCS TestPAPageMoveCommand.cpp )

kde4_add_unit_test(TestPAPageMoveCommand TESTNAME libs-kopageapp-TestPAPageMoveCommand ${TestPAPageMoveCommand_test_SRCS})

target_link_libraries(TestPAPageMoveCommand   kopageapp ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPACopyPastePage_test_SRCS TestPACopyPastePage.cpp )

kde4_add_unit_test(TestPACopyPastePage TESTNAME libs-kopageapp-TestPACopyPastePage ${TestPACopyPastePage_test_SRCS})

target_link_libraries(TestPACopyPastePage   kopageapp ${QT_QTTEST_LIBRARY})

########### next target ###############
