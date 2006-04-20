kde4_header()

include_directories( ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(priorityqueue_test_SRCS priorityqueue_test.cpp )

kde4_automoc(${priorityqueue_test_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(priorityqueue_test ${priorityqueue_test_SRCS})

target_link_libraries(priorityqueue_test  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(filterchain_test_SRCS filterchain_test.cpp )

kde4_automoc(${filterchain_test_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(filterchain_test ${filterchain_test_SRCS})

target_link_libraries(filterchain_test  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(filter_graph_SRCS filter_graph.cpp )

kde4_automoc(${filter_graph_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(filter_graph ${filter_graph_SRCS})

target_link_libraries(filter_graph  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kogenstylestest_SRCS kogenstylestest.cpp )

kde4_automoc(${kogenstylestest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(kogenstylestest ${kogenstylestest_SRCS})

target_link_libraries(kogenstylestest  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kodomtest_SRCS kodomtest.cpp )

kde4_automoc(${kodomtest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(kodomtest ${kodomtest_SRCS})

target_link_libraries(kodomtest  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kooasissettingstest_SRCS kooasissettingstest.cpp )

kde4_automoc(${kooasissettingstest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(kooasissettingstest ${kooasissettingstest_SRCS})

target_link_libraries(kooasissettingstest  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kooasisstoretest_SRCS kooasisstoretest.cpp )

kde4_automoc(${kooasisstoretest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(kooasisstoretest ${kooasisstoretest_SRCS})

target_link_libraries(kooasisstoretest  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kopointtest_SRCS kopointtest.cpp )

kde4_automoc(${kopointtest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(kopointtest ${kopointtest_SRCS})

target_link_libraries(kopointtest  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(korecttest_SRCS korecttest.cpp )

kde4_automoc(${korecttest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(korecttest ${korecttest_SRCS})

target_link_libraries(korecttest  ${KDE4_KDECORE_LIBS} kofficecore )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

######## General stuff
#
#INCLUDES= -I$(srcdir)/.. $(KSTORE_INCLUDES) $(all_includes)
#
######## Files
#
#check_PROGRAMS = priorityqueue_test filterchain_test filter_graph kogenstylestest \
#	kodomtest kooasissettingstest kooasisstoretest kopointtest korecttest
#
#TESTS = kogenstylestest kodomtest kooasissettingstest kooasisstoretest kopointtest korecttest
#
#priorityqueue_test_SOURCES = priorityqueue_test.cpp
#priorityqueue_test_LDADD = ../libkofficecore.la
#
#filterchain_test_SOURCES = filterchain_test.cpp
#filterchain_test_LDADD = ../libkofficecore.la
#
#filter_graph_SOURCES = filter_graph.cpp
#filter_graph_LDADD = ../libkofficecore.la
#
#kogenstylestest_SOURCES = kogenstylestest.cpp
#kogenstylestest_LDADD = ../libkofficecore.la
#
#kodomtest_SOURCES = kodomtest.cpp
#kodomtest_LDADD = ../libkofficecore.la
#
#kooasissettingstest_SOURCES = kooasissettingstest.cpp
#kooasissettingstest_LDADD = ../libkofficecore.la
#
#kooasisstoretest_SOURCES = kooasisstoretest.cpp
#kooasisstoretest_LDADD = ../libkofficecore.la
#
#kopointtest_SOURCES = kopointtest.cpp
#kopointtest_LDADD = ../libkofficecore.la
#
#korecttest_SOURCES = korecttest.cpp
#korecttest_LDADD = ../libkofficecore.la
#
#dot:
#	dot -Tpng -o graph.png graph.dot
