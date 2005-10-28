kde_services_DATA = kritacmykplugin.desktop

INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../kritacolor/color_strategy \
	-I$(srcdir)/../../kritacolor \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritacmykplugin.la

kritacmykplugin_la_SOURCES = cmyk_plugin.cc kis_cmyk_colorspace.cc
noinst_HEADERS = cmyk_plugin.h kis_cmyk_colorspace.h 

kritacmykplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacmykplugin_la_LIBADD =  ../../kritacolor/libkritacolor.la

kritacmykplugin_la_METASOURCES = AUTO


SUBDIRS=templates
