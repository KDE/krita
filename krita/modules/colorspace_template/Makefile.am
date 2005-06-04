#kritarcdir = $(kde_datadir)/krita/kpartplugins
#kritarc_DATA = templateplugin.rc
kde_services_DATA = kritatemplateplugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritatemplateplugin.la

kritatemplateplugin_la_SOURCES = template_plugin.cc kis_colorspace_template.cc
noinst_HEADERS = template_plugin.h kis_colorspace_template.h

kritatemplateplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatemplateplugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatemplateplugin_la_METASOURCES = AUTO

SUBDIRS=templates
