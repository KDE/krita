# location for the rc file
#kritarcdir = $(kde_datadir)/krita/kpartplugins

# rc file containing the GUI for the plugin
#kritarc_DATA = rgb_u16_plugin.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = krita_rgb_u16_plugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_rgb_u16_plugin.la

# Srcs for the plugin
krita_rgb_u16_plugin_la_SOURCES = rgb_u16_plugin.cc kis_strategy_colorspace_rgb_u16.cc
noinst_HEADERS = rgb_u16_plugin.h kis_strategy_colorspace_rgb_u16.h

krita_rgb_u16_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_rgb_u16_plugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

krita_rgb_u16_plugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?

if include_kunittest_tests
TESTSDIR = tests

# Build a library to link the tests against to avoid warnings about linking to a module.
check_LTLIBRARIES = libkrita_rgb_u16_plugin.la
libkrita_rgb_u16_plugin_la_SOURCES = rgb_u16_plugin.cc kis_strategy_colorspace_rgb_u16.cc
libkrita_rgb_u16_plugin_la_LDFLAGS = $(all_libraries)
libkrita_rgb_u16_plugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

endif

SUBDIRS = . $(TESTSDIR)

