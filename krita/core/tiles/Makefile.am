if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . $(TESTSDIR)

INCLUDES  = -I$(srcdir)/../ \
	-I$(srcdir)/../../sdk \
    $(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(all_includes)

noinst_LTLIBRARIES = libkritatile.la

libkritatile_la_SOURCES = kis_tiledvlineiterator.cc kis_tiledhlineiterator.cc \
		kis_tileddatamanager.cc kis_tile.cc kis_tilediterator.cc kis_tiledrectiterator.cc \
		kis_memento.cc kis_tilemanager.cc kis_tiled_random_accessor.cc
# kis_tiled_random_accessor.cc
libkritatile_la_METASOURCES = AUTO

include_HEADERS = \
    kis_tileddatamanager.h
noinst_HEADERS = kis_tiled_random_accessor.h
