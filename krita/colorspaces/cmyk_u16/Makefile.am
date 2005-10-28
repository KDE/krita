kde_services_DATA = krita_cmyk_u16_plugin.desktop

INCLUDES  =  -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../kritacolor/ \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

lib_LTLIBRARIES = libkrita_cmyk_u16.la
libkrita_cmyk_u16_la_SOURCES = kis_cmyk_u16_colorspace.cc
libkrita_cmyk_u16_la_LDFLAGS = $(all_libraries)
libkrita_cmyk_u16_la_LIBADD =  ../../kritacolor/libkritacolor.la

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_cmyk_u16_plugin.la

# Srcs for the plugin
krita_cmyk_u16_plugin_la_SOURCES = cmyk_u16_plugin.cc
noinst_HEADERS = cmyk_u16_plugin.h kis_cmyk_u16_colorspace.h

krita_cmyk_u16_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_cmyk_u16_plugin_la_LIBADD = libkrita_cmyk_u16.la ../../kritacolor/libkritacolor.la

krita_cmyk_u16_plugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?


SUBDIRS = .
#$(TESTSDIR)

