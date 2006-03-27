include $(top_srcdir)/lib/kross/Makefile.global

noinst_PROGRAMS = krosstest

krosstest_SOURCES = testobject.cpp testaction.cpp testplugin.cpp testwindow.cpp main.cpp

krosstest_LDADD = \
    $(LIB_QT) \
    $(LIB_KDECORE) \
    $(LIB_KDEUI) \
    ../api/libkrossapi.la \
    ../main/libkrossmain.la

INCLUDES = $(KROSS_INCLUDES) $(all_includes)
krosstest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
SUBDIRS = .
METASOURCES = AUTO
