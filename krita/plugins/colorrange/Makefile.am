kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = colorrange.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core -I$(srcdir)/../../core/compositeop  -I$(srcdir)/../../core/color_strategy -I$(srcdir)/../../core/tiles -I$(srcdir)/../../ui -I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritacolorrange.la

kritacolorrange_la_SOURCES = colorrange.cc dlg_colorrange.cc wdg_colorrange.ui 
noinst_HEADERS = wdg_colorrange.h dlg_colorrange.h colorrange.h


kritacolorrange_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacolorrange_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritacolorrange_la_METASOURCES = AUTO
