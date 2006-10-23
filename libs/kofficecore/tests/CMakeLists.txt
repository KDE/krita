

if(KDE4_BUILD_TESTS)

include_directories( ${KOFFICECORE_INCLUDES} )

########### next target ###############

set(priorityqueue_test_SRCS priorityqueue_test.cpp )

kde4_automoc(${priorityqueue_test_SRCS})

kde4_add_executable(priorityqueue_test RUN_UNINSTALLED ${priorityqueue_test_SRCS})

target_link_libraries(priorityqueue_test  ${KDE4_KDECORE_LIBS} kofficecore )

########### next target ###############

set(filterchain_test_SRCS filterchain_test.cpp )

kde4_automoc(${filterchain_test_SRCS})

kde4_add_executable(filterchain_test RUN_UNINSTALLED ${filterchain_test_SRCS})

target_link_libraries(filterchain_test  ${KDE4_KDECORE_LIBS} kofficecore )

########### next target ###############

set(filter_graph_SRCS filter_graph.cpp )

kde4_automoc(${filter_graph_SRCS})

kde4_add_executable(filter_graph RUN_UNINSTALLED ${filter_graph_SRCS})

target_link_libraries(filter_graph  ${KDE4_KDECORE_LIBS} kofficecore )

########### next target ###############

set(kogenstylestest_SRCS kogenstylestest.cpp )

kde4_automoc(${kogenstylestest_SRCS})

kde4_add_executable(kogenstylestest RUN_UNINSTALLED ${kogenstylestest_SRCS})

target_link_libraries(kogenstylestest  ${KDE4_KDECORE_LIBS} kofficecore )

add_test(kofficecore-kogenstylestest kogenstylestest)

########### next target ###############

set(kooasissettingstest_SRCS kooasissettingstest.cpp )

kde4_automoc(${kooasissettingstest_SRCS})

kde4_add_executable(kooasissettingstest RUN_UNINSTALLED ${kooasissettingstest_SRCS})

target_link_libraries(kooasissettingstest  ${KDE4_KDECORE_LIBS} kofficecore )

add_test(kofficecore-kooasissettingstest kooasissettingstest)

########### next target ###############

set(kooasisstoretest_SRCS kooasisstoretest.cpp )

kde4_automoc(${kooasisstoretest_SRCS})

kde4_add_executable(kooasisstoretest RUN_UNINSTALLED ${kooasisstoretest_SRCS})

target_link_libraries(kooasisstoretest  ${KDE4_KDECORE_LIBS} kofficecore )

add_test(kofficecore-kooasisstoretest kooasisstoretest)

########### next target ###############

set(kopointtest_SRCS kopointtest.cpp )

kde4_automoc(${kopointtest_SRCS})

kde4_add_executable(kopointtest RUN_UNINSTALLED ${kopointtest_SRCS})

target_link_libraries(kopointtest  ${KDE4_KDECORE_LIBS} kofficecore )

add_test(kofficecore-kopointtest kopointtest)

########### next target ###############

set(korecttest_SRCS korecttest.cpp )

kde4_automoc(${korecttest_SRCS})

kde4_add_executable(korecttest RUN_UNINSTALLED ${korecttest_SRCS})

target_link_libraries(korecttest kofficecore ${QT_QTTEST_LIBRARY} )

add_test(kofficecore-korecttest korecttest)

########### next target ###############

set(rtreetestapp_SRCS rtreetestapp.cpp Tool.cpp )

kde4_automoc(${rtreetestapp_SRCS})

kde4_add_executable(rtreetestapp RUN_UNINSTALLED ${rtreetestapp_SRCS})

target_link_libraries(rtreetestapp kofficecore ${QT_QTTEST_LIBRARY} )

########### next target ###############

set(koxmlreader_SRCS koxmlreadertest.cpp )

kde4_automoc(${koxmlreader_SRCS})

kde4_add_executable(koxmlreadertest RUN_UNINSTALLED ${koxmlreader_SRCS})

target_link_libraries(koxmlreadertest kofficecore ${QT_QTTEST_LIBRARY} )

add_test(kofficecore-koxmlreadertest koxmlreadertest)


########### end ###############

endif(KDE4_BUILD_TESTS)
