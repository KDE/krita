####### General stuff

KDE_CXXFLAGS = $(USE_RTTI)
INCLUDES= -I$(srcdir)/.. $(all_includes)

####### Files

check_PROGRAMS = storage_test xmlwritertest storedroptest
TESTS = storage_test xmlwritertest

storage_test_SOURCES = storage_test.cpp
storage_test_LDADD = ../libkstore.la

xmlwritertest_SOURCES = xmlwritertest.cpp
xmlwritertest_LDADD = ../libkstore.la

storedroptest_SOURCES = storedroptest.cpp
storedroptest_LDADD = ../libkstore.la
