# Install the desktop file needed to detect the plugin
kde_services_DATA = krita_rgb_f32_plugin.desktop

INCLUDES  = \
	-I$(srcdir)/../../sdk \
	-I$(srcdir)/../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../kritacolor/ \
	$(KOFFICE_INCLUDES) \
	$(all_includes)


lib_LTLIBRARIES = libkrita_rgb_f32.la
libkrita_rgb_f32_la_SOURCES = kis_rgb_f32_colorspace.cc
libkrita_rgb_f32_la_LDFLAGS = $(all_libraries)
libkrita_rgb_f32_la_LIBADD =  ../../kritacolor/libkritacolor.la

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_rgb_f32_plugin.la

# Srcs for the plugin
krita_rgb_f32_plugin_la_SOURCES = rgb_f32_plugin.cc
noinst_HEADERS = rgb_f32_plugin.h kis_rgb_f32_colorspace.h

krita_rgb_f32_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_rgb_f32_plugin_la_LIBADD =  libkrita_rgb_f32.la ../../kritacolor/libkritacolor.la

krita_rgb_f32_plugin_la_METASOURCES = AUTO

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . $(TESTSDIR)

