#kritarcdir = $(kde_datadir)/krita/kpartplugins
#kritarc_DATA = cmykplugin.rc
kde_services_DATA = kritacmykplugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritacmykplugin.la

kritacmykplugin_la_SOURCES = cmyk_plugin.cc kis_cmyk_colorspace.cc
noinst_HEADERS = cmyk_plugin.h kis_cmyk_colorspace.h 

kritacmykplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacmykplugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritacmykplugin_la_METASOURCES = AUTO


SUBDIRS=templates
