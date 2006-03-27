INCLUDES = $(KOFFICECORE_INCLUDES) $(KOFFICEUI_INCLUDES) $(all_includes)
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

METASOURCES = AUTO

noinst_HEADERS = hnjalloc.h hyphen.h kohyphen.h

noinst_LTLIBRARIES = libkohyphen.la
libkohyphen_la_SOURCES = hnjalloc.c hyphen.c kohyphen.cpp
libkohyphen_la_LIBADD = $(LIB_KDECORE)
libkohyphen_la_LDFLAGS = $(all_libraries) -no-undefined

dictsdir = $(kde_datadir)/koffice/hyphdicts

SUBDIRS = . hyphdicts

check_PROGRAMS = kohyphentest

kohyphentest_SOURCES = kohyphentest.cpp
kohyphentest_LDADD = ./libkohyphen.la $(LIB_QT) $(LIB_KDECORE)
kohyphentest_LDFLAGS = $(KDE_RPATH) $(all_libraries)
