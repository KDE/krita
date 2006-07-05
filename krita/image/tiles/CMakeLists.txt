

if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)

########### next target ###############


########### install files ###############

install( FILES  kis_tileddatamanager.h  DESTINATION ${INCLUDE_INSTALL_DIR})





#original Makefile.am contents follow:

#if include_kunittest_tests
#TESTSDIR = tests
#endif
#
#SUBDIRS = . $(TESTSDIR)
#
#INCLUDES  = -I$(srcdir)/../ \
#	-I$(srcdir)/../../sdk \
#    $(KOFFICE_INCLUDES) \
#	-I$(interfacedir) \
#	$(all_includes)
#
#noinst_LTLIBRARIES = libkritatile.la
#
#libkritatile_la_SOURCES = kis_tiledvlineiterator.cc kis_tiledhlineiterator.cc kis_tileddatamanager.cc kis_tile.cc kis_tilediterator.cc kis_tiledrectiterator.cc kis_memento.cc kis_tilemanager.cc 
#libkritatile_la_METASOURCES = AUTO
#
#include_HEADERS = \
#    kis_tileddatamanager.h
