kde_services_DATA = kritawsplugin.desktop
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

kde_module_LTLIBRARIES = kritawsplugin.la

kritawsplugin_la_SOURCES = wet_sticky_plugin.cc kis_colorspace_wet_sticky.cc
noinst_HEADERS = wet_sticky_plugin.h kis_colorspace_wet_sticky.h

kritawsplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritawsplugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritawsplugin_la_METASOURCES = AUTO

SUBDIRS=filter


