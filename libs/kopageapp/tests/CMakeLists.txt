set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOPAGEAPP_INCLUDES} )

########### next target ###############

set(TestPAPageInsertCommand_test_SRCS TestPAPageInsertCommand.cpp )

add_executable(TestPAPageInsertCommandNAME libs-kopageapp-TestPAPageInsertCommand ${TestPAPageInsertCommand_test_SRCS})
add_test(TestPAPageInsertCommand TestPAPageInsertCommand)
ecm_mark_as_test(TestPAPageInsertCommand)

target_link_libraries(TestPAPageInsertCommand   kopageapp Qt5::Test)

########### next target ###############

set(TestPAPageDeleteCommand_test_SRCS TestPAPageDeleteCommand.cpp )

add_executable(TestPAPageDeleteCommandNAME libs-kopageapp-TestPAPageDeleteCommand ${TestPAPageDeleteCommand_test_SRCS})
add_test(TestPAPageDeleteCommand TestPAPageDeleteCommand)
ecm_mark_as_test(TestPAPageDeleteCommand)

target_link_libraries(TestPAPageDeleteCommand   kopageapp Qt5::Test)

########### next target ###############

set(TestPAPageMoveCommand_test_SRCS TestPAPageMoveCommand.cpp )

add_executable(TestPAPageMoveCommandNAME libs-kopageapp-TestPAPageMoveCommand ${TestPAPageMoveCommand_test_SRCS})
add_test(TestPAPageMoveCommand TestPAPageMoveCommand)
ecm_mark_as_test(TestPAPageMoveCommand)

target_link_libraries(TestPAPageMoveCommand   kopageapp Qt5::Test)

########### next target ###############

set(TestPACopyPastePage_test_SRCS TestPACopyPastePage.cpp )

add_executable(TestPACopyPastePageNAME libs-kopageapp-TestPACopyPastePage ${TestPACopyPastePage_test_SRCS})
add_test(TestPACopyPastePage TestPACopyPastePage)
ecm_mark_as_test(TestPACopyPastePage)

target_link_libraries(TestPACopyPastePage   kopageapp Qt5::Test)

########### next target ###############
