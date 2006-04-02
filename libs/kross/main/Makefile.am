include $(top_srcdir)/libs/kross/Makefile.global

lib_LTLIBRARIES = libkrossmain.la

libkrossmain_la_SOURCES = mainmodule.cpp scriptcontainer.cpp manager.cpp \
	scriptaction.cpp scriptguiclient.cpp wdgscriptsmanagerbase.ui wdgscriptsmanager.cpp

libkrossmain_la_LDFLAGS = $(all_libraries) $(VER_INFO) -Wnounresolved

mainincludedir=$(includedir)/kross/main

maininclude_HEADERS = \
    krossconfig.h \
    mainmodule.h \
    manager.h \
    scriptaction.h \
    scriptcontainer.h \
    scriptguiclient.h \
    wdgscriptsmanager.h \
    wdgscriptsmanagerbase.h 

libkrossmain_la_LIBADD = \
    $(LIB_QT) \
    $(LIB_KDECORE) \
    $(LIB_KFILE) \
    $(LIB_KDEUI) \
    $(LIB_KNEWSTUFF) \
    $(LIB_KROSS_API)

METASOURCES = AUTO
SUBDIRS = .
INCLUDES = $(KROSS_INCLUDES) $(all_includes)
noinst_HEADERS = wdgscriptsmanager.h
