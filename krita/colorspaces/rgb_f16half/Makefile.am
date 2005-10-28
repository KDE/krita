# Install the desktop file needed to detect the plugin
kde_services_DATA = krita_rgb_f16half_plugin.desktop

INCLUDES  = \
	-I$(srcdir)/../../sdk \
	-I$(srcdir)/../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../kritacolor/ \
	$(OPENEXR_CFLAGS) \
	$(KOFFICE_INCLUDES) \
	$(all_includes)


lib_LTLIBRARIES = libkrita_rgb_f16half.la
libkrita_rgb_f16half_la_SOURCES = kis_rgb_f16half_colorspace.cc
libkrita_rgb_f16half_la_LDFLAGS = $(all_libraries)
libkrita_rgb_f16half_la_LIBADD =  ../../kritacolor/libkritacolor.la

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_rgb_f16half_plugin.la

# Srcs for the plugin
krita_rgb_f16half_plugin_la_SOURCES = rgb_f16half_plugin.cc
noinst_HEADERS = rgb_f16half_plugin.h kis_rgb_f16half_colorspace.h

krita_rgb_f16half_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_rgb_f16half_plugin_la_LIBADD = libkrita_rgb_f16half.la ../../kritacolor/libkritacolor.la

krita_rgb_f16half_plugin_la_METASOURCES = AUTO

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . $(TESTSDIR)

