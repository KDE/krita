kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = variations.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritavariations.la

kritavariations_la_SOURCES = variations.cc dlg_variations.cc wdg_variations.ui 
noinst_HEADERS = wdg_variations.h

kritavariations_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritavariations_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritavariations_la_METASOURCES = AUTO
