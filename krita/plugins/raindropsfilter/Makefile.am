kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = raindropsfilter.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../ui \
	-I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritaraindropsfilter.la

kritaraindropsfilter_la_SOURCES = wdg_raindropsfilter.ui raindropsfilter.cc dlg_raindropsfilter.cc
noinst_HEADERS = wdg_raindropsfilter.h dlg_raindropsfilter.h raindropsfilter.h

kritaraindropsfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaraindropsfilter_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritaraindropsfilter_la_METASOURCES = AUTO
