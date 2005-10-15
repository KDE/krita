# Install the desktop file needed to detect the plugin
kde_services_DATA = krita_rgb_f16half_plugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = \
	-I$(srcdir)/../../sdk \
	-I$(srcdir)/../../libkritacolor/color_strategy/ \
	-I$(srcdir)/../../libkritacolor/ \
	$(OPENEXR_CFLAGS) \
	$(all_includes)


lib_LTLIBRARIES = libkrita_rgb_f16half.la
libkrita_rgb_f16half_la_SOURCES = kis_rgb_f16half_colorspace.cc
libkrita_rgb_f16half_la_LDFLAGS = $(all_libraries)
libkrita_rgb_f16half_la_LIBADD =  ../../libkritacolor/libkritacolor.la

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_rgb_f16half_plugin.la

# Srcs for the plugin
krita_rgb_f16half_plugin_la_SOURCES = rgb_f16half_plugin.cc
noinst_HEADERS = rgb_f16half_plugin.h kis_rgb_f16half_colorspace.h

krita_rgb_f16half_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_rgb_f16half_plugin_la_LIBADD = libkrita_rgb_f16half.la ../../libkritacolor/libkritacolor.la

krita_rgb_f16half_plugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . $(TESTSDIR)

