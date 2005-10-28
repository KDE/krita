# Install the desktop file needed to detect the plugin
kde_services_DATA = kritargbplugin.desktop

INCLUDES  =  -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../kritacolor/ \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

lib_LTLIBRARIES = libkritargb.la
libkritargb_la_SOURCES = kis_rgb_colorspace.cc
libkritargb_la_LDFLAGS = $(all_libraries)
libkritargb_la_LIBADD =  ../../kritacolor/libkritacolor.la

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritargbplugin.la

# Srcs for the plugin
kritargbplugin_la_SOURCES = rgb_plugin.cc
noinst_HEADERS = rgb_plugin.h kis_rgb_colorspace.h

kritargbplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritargbplugin_la_LIBADD =  libkritargb.la ../../kritacolor/libkritacolor.la

kritargbplugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . templates $(TESTSDIR)

