include $(top_srcdir)/lib/kross/Makefile.global

#noinst_PROGRAMS = krossrunner
bin_PROGRAMS = krossrunner

krossrunner_SOURCES = main.cpp
krossrunner_LDADD = $(LIB_QT) $(LIB_KDECORE) ../api/libkrossapi.la ../main/libkrossmain.la
INCLUDES = $(KROSS_INCLUDES) $(all_includes)
krossrunner_LDFLAGS = $(all_libraries) $(KDE_RPATH)
SUBDIRS = .
METASOURCES = AUTO
