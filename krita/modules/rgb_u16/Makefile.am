# location for the rc file
kde_services_DATA = krita_rgb_u16_plugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

lib_LTLIBRARIES = libkrita_rgb_u16.la
libkrita_rgb_u16_la_SOURCES = kis_rgb_u16_colorspace.cc
libkrita_rgb_u16_la_LDFLAGS = $(all_libraries)
libkrita_rgb_u16_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_rgb_u16_plugin.la

# Srcs for the plugin
krita_rgb_u16_plugin_la_SOURCES = rgb_u16_plugin.cc
noinst_HEADERS = rgb_u16_plugin.h kis_rgb_u16_colorspace.h

krita_rgb_u16_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_rgb_u16_plugin_la_LIBADD =  ../../libkritacommon.la libkrita_rgb_u16.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

krita_rgb_u16_plugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . $(TESTSDIR)

