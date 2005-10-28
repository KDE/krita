kde_services_DATA = kritawsplugin.desktop
INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../../color_strategy/ \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritawsplugin.la

kritawsplugin_la_SOURCES = wet_sticky_plugin.cc kis_wet_sticky_colorspace.cc kis_ws_engine_filter.cc
noinst_HEADERS = wet_sticky_plugin.h kis_wet_sticky_colorspace.h kis_ws_engine_filter.h

kritawsplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritawsplugin_la_LIBADD =  ../../kritacolor/libkritacolor.la

kritawsplugin_la_METASOURCES = AUTO

SUBDIRS=brushop


