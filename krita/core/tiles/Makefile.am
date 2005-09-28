if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . $(TESTSDIR)

INCLUDES  = -I$(srcdir)/../ \
	-I$(srcdir)/../../sdk \
	-I$(interfacedir) \
	$(all_includes)

noinst_LTLIBRARIES = libkistile.la

libkistile_la_SOURCES = kis_tiledvlineiterator.cc kis_tiledhlineiterator.cc kis_tileddatamanager.cc kis_tile.cc kis_tilediterator.cc kis_tiledrectiterator.cc kis_memento.cc kis_tilemanager.cc 
libkistile_la_METASOURCES = AUTO

