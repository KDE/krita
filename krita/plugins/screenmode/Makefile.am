kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = screenmode.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core -I$(srcdir)/../../core/tiles -I$(srcdir)/../ui -I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = screenmode.la

screenmode_la_SOURCES = screenmode.cc 
noinst_HEADERS = screenmode.h

screenmode_la_LDFLAGS = $(all_libraries) -avoid-version $(KDE_PLUGIN)
screenmode_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

screenmode_la_METASOURCES = AUTO
