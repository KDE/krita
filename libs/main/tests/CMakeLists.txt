include_directories( ${KOMAIN_INCLUDES} )

########### next target ###############

set(priorityqueue_test_SRCS priorityqueue_test.cpp )

kde4_automoc(${priorityqueue_test_SRCS})

kde4_add_unit_test(priorityqueue_test TESTNAME libs-komain-priorityQueueTest ${priorityqueue_test_SRCS})

target_link_libraries(priorityqueue_test  ${KDE4_KDECORE_LIBS} komain )

########### next target ###############

set(filterchain_test_SRCS filterchain_test.cpp )

kde4_automoc(${filterchain_test_SRCS})

kde4_add_unit_test(filterchain_test TESTNAME libs-komain-filterchainTest ${filterchain_test_SRCS})

target_link_libraries(filterchain_test  ${KDE4_KDECORE_LIBS} komain )

########### next target ###############

set(filter_graph_SRCS filter_graph.cpp )

kde4_automoc(${filter_graph_SRCS})

kde4_add_unit_test(filter_graph TESTNAME libs-komain-filterGraphTest ${filter_graph_SRCS})

target_link_libraries(filter_graph  ${KDE4_KDECORE_LIBS} komain )

########### next target ###############

set(kogenstylestest_SRCS kogenstylestest.cpp )

kde4_automoc(${kogenstylestest_SRCS})

kde4_add_unit_test(kogenstylestest TESTNAME libs-komain-kogenstylestest ${kogenstylestest_SRCS})

target_link_libraries(kogenstylestest  ${KDE4_KDECORE_LIBS} komain )

########### next target ###############

set(kooasissettingstest_SRCS kooasissettingstest.cpp )

kde4_automoc(${kooasissettingstest_SRCS})

kde4_add_unit_test(kooasissettingstest TESTNAME libs-komain-kooasissettingstest ${kooasissettingstest_SRCS})

target_link_libraries(kooasissettingstest  ${KDE4_KDECORE_LIBS} komain )

########### next target ###############

set(kooasisstoretest_SRCS kooasisstoretest.cpp )

kde4_automoc(${kooasisstoretest_SRCS})

kde4_add_unit_test(kooasisstoretest TESTNAME libs-komain-kooasisstoretest ${kooasisstoretest_SRCS})

target_link_libraries(kooasisstoretest  ${KDE4_KDECORE_LIBS} komain )

########### next target ###############

if(KDE4_BUILD_TESTS)
set(rtreetestapp_SRCS rtreetestapp.cpp Tool.cpp )

kde4_automoc(${rtreetestapp_SRCS})

# this test shows a GUI and expects interaction; so don't run it with a 'make test'
kde4_add_executable(rtreetestapp RUN_UNINSTALLED ${rtreetestapp_SRCS})

target_link_libraries(rtreetestapp komain ${QT_QTTEST_LIBRARY} )
endif(KDE4_BUILD_TESTS)

########### end ###############
