kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = histogram.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../ui -I../../ui \
	-I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = histogram.la

histogram_la_SOURCES = histogram.cc dlg_histogram.cc 
noinst_HEADERS = dlg_histogram.h histogram.h


histogram_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
histogram_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

histogram_la_METASOURCES = AUTO
