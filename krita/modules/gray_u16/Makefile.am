# location for the rc file
kde_services_DATA = krita_gray_u16_plugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

lib_LTLIBRARIES = libkrita_gray_u16.la
libkrita_gray_u16_la_SOURCES = kis_gray_u16_colorspace.cc
libkrita_gray_u16_la_LDFLAGS = $(all_libraries)
libkrita_gray_u16_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_gray_u16_plugin.la

# Srcs for the plugin
krita_gray_u16_plugin_la_SOURCES = gray_u16_plugin.cc
noinst_HEADERS = gray_u16_plugin.h kis_gray_u16_colorspace.h

krita_gray_u16_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_gray_u16_plugin_la_LIBADD =  ../../libkritacommon.la libkrita_gray_u16.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

krita_gray_u16_plugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?


SUBDIRS = .
#$(TESTSDIR)

