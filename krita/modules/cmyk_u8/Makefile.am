#kritarcdir = $(kde_datadir)/krita/kpartplugins
#kritarc_DATA = cmykplugin.rc
kde_services_DATA = kritacmykplugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../libkritacolor \
	$(all_includes)

kde_module_LTLIBRARIES = kritacmykplugin.la

kritacmykplugin_la_SOURCES = cmyk_plugin.cc kis_cmyk_colorspace.cc
noinst_HEADERS = cmyk_plugin.h kis_cmyk_colorspace.h 

kritacmykplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacmykplugin_la_LIBADD =  ../../libkritacolor/libkritacolor.la

kritacmykplugin_la_METASOURCES = AUTO


SUBDIRS=templates
