kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = grayplugin.rc
kde_services_DATA = kritagrayplugin.desktop

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../ui \
	-I$(srcdir)../../tools \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritagrayplugin.la

kritagrayplugin_la_SOURCES = gray_plugin.cc kis_strategy_colorspace_grayscale.cc
noinst_HEADERS = gray_plugin.h kis_strategy_colorspace_grayscale.h

kritagrayplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritagrayplugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritagrayplugin_la_METASOURCES = AUTO


