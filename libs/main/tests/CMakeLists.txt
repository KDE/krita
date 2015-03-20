set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOMAIN_INCLUDES} )

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

########### next target ###############

set(priorityqueue_test_SRCS priorityqueue_test.cpp )
add_executable(priorityqueue_testNAME libs-komain-priorityQueueTest ${priorityqueue_test_SRCS})
add_test(priorityqueue_test priorityqueue_test)
ecm_mark_as_test(priorityqueue_test)
target_link_libraries(priorityqueue_test Qt5::Test )

########### next target ###############

set(filterchain_test_SRCS filterchain_test.cpp )
add_executable(filterchain_testNAME libs-komain-filterchainTest ${filterchain_test_SRCS})
add_test(filterchain_test filterchain_test)
ecm_mark_as_test(filterchain_test)
target_link_libraries(filterchain_test   komain )

########### next target ###############

set(filter_graph_SRCS filter_graph.cpp )
add_executable(filter_graphNAME libs-komain-filterGraphTest ${filter_graph_SRCS})
add_test(filter_graph filter_graph)
ecm_mark_as_test(filter_graph)
target_link_libraries(filter_graph   komain )

########### next target ###############


set(rtreetestapp_SRCS rtreetestapp.cpp Tool.cpp )
add_executable(rtreetestapp TEST ${rtreetestapp_SRCS})
target_link_libraries(rtreetestapp komain Qt5::Test )

########### next target ###############

set(documentstructuretest_SRCS documentstructuretest.cpp )
add_executable(DocumentStructureTestNAME libs-komain-DocumentStructureTest ${documentstructuretest_SRCS})
add_test(DocumentStructureTest DocumentStructureTest)
ecm_mark_as_test(DocumentStructureTest)
target_link_libraries(DocumentStructureTest komain  Qt5::Test)

########### next target ###############

set(findmatch_test_SRCS testfindmatch.cpp )
add_executable(testfindmatchNAME libs-komain-testfindmatch ${findmatch_test_SRCS})
add_test(testfindmatch testfindmatch)
ecm_mark_as_test(testfindmatch)
target_link_libraries(testfindmatch komain Qt5::Test)

