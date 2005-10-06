# location for the rc file
kde_services_DATA = krita_cmyk_u16_plugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

lib_LTLIBRARIES = libkrita_cmyk_u16.la
libkrita_cmyk_u16_la_SOURCES = kis_cmyk_u16_colorspace.cc
libkrita_cmyk_u16_la_LDFLAGS = $(all_libraries)
libkrita_cmyk_u16_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_cmyk_u16_plugin.la

# Srcs for the plugin
krita_cmyk_u16_plugin_la_SOURCES = cmyk_u16_plugin.cc
noinst_HEADERS = cmyk_u16_plugin.h kis_cmyk_u16_colorspace.h

krita_cmyk_u16_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_cmyk_u16_plugin_la_LIBADD =  ../../libkritacommon.la libkrita_cmyk_u16.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

krita_cmyk_u16_plugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?


SUBDIRS = .
#$(TESTSDIR)

