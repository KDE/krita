kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritaexample.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../core -I$(srcdir)/../../core/color_strategy -I$(srcdir)/../../core/tiles -I$(srcdir)/../ui -I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritaexample.la

kritaexample_la_SOURCES = example.cc
noinst_HEADERS = example.h

kritaexample_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaexample_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritaexample_la_METASOURCES = AUTO
