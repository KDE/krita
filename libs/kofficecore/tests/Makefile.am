####### General stuff

INCLUDES= -I$(srcdir)/.. $(KSTORE_INCLUDES) $(all_includes)

####### Files

check_PROGRAMS = priorityqueue_test filterchain_test filter_graph kogenstylestest \
	kodomtest kooasissettingstest kooasisstoretest kopointtest korecttest

TESTS = kogenstylestest kodomtest kooasissettingstest kooasisstoretest kopointtest korecttest

priorityqueue_test_SOURCES = priorityqueue_test.cpp
priorityqueue_test_LDADD = ../libkofficecore.la

filterchain_test_SOURCES = filterchain_test.cpp
filterchain_test_LDADD = ../libkofficecore.la

filter_graph_SOURCES = filter_graph.cpp
filter_graph_LDADD = ../libkofficecore.la

kogenstylestest_SOURCES = kogenstylestest.cpp
kogenstylestest_LDADD = ../libkofficecore.la

kodomtest_SOURCES = kodomtest.cpp
kodomtest_LDADD = ../libkofficecore.la

kooasissettingstest_SOURCES = kooasissettingstest.cpp
kooasissettingstest_LDADD = ../libkofficecore.la

kooasisstoretest_SOURCES = kooasisstoretest.cpp
kooasisstoretest_LDADD = ../libkofficecore.la

kopointtest_SOURCES = kopointtest.cpp
kopointtest_LDADD = ../libkofficecore.la

korecttest_SOURCES = korecttest.cpp
korecttest_LDADD = ../libkofficecore.la

dot:
	dot -Tpng -o graph.png graph.dot
