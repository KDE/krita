kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = variations.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core -I$(srcdir)/../../core/tiles -I$(srcdir)/../ui -I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = variations.la

variations_la_SOURCES = variations.cc dlg_variations.cc wdg_variations.ui 
noinst_HEADERS = wdg_variations.h

variations_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
variations_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

variations_la_METASOURCES = AUTO
