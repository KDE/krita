kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = cmykplugin.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../ui \
	-I$(srcdir)../../tools \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = cmykplugin.la

cmykplugin_la_SOURCES = cmyk_plugin.cc kis_strategy_colorspace_cmyk.cc
noinst_HEADERS = cmyk_plugin.h kis_strategy_colorspace_cmyk.h

cmykplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
cmykplugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

cmykplugin_la_METASOURCES = AUTO
