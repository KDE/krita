INCLUDES = $(KOFFICECORE_INCLUDES) $(KOFFICEUI_INCLUDES) $(KOTEXT_INCLUDES) $(all_includes)
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

METASOURCES = AUTO

check_PROGRAMS = kotextformattertest kobordertest kovariabletest

TESTS = kotextformattertest kovariabletest

LDADD = ../libkotext.la
AM_LDFLAGS = $(KDE_RPATH) $(all_libraries)

kobordertest_SOURCES = kobordertest.cpp
kotextformattertest_SOURCES = kotextformattertest.cpp
kovariabletest_SOURCES = kovariabletest.cpp

check_LTLIBRARIES = kunittest_KoUserStyleTester.la 

kunittest_KoUserStyleTester_la_SOURCES = KoUserStyleTester.cpp
kunittest_KoUserStyleTester_la_LIBADD = ../libkotext.la -lkunittest
kunittest_KoUserStyleTester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

check-local:
	kunittestmodrunner
