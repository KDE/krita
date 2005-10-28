# Install the desktop file needed to detect the plugin
kde_services_DATA = kritatestcsplugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../../color_strategy/ \
	-I$(interfacedir) \
	$(all_includes)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatestcsplugin.la

# Srcs for the plugin
kritatestcsplugin_la_SOURCES = testcs_plugin.cc kis_test_colorspace.cc
noinst_HEADERS = testcs_plugin.h kis_test_colorspace.h

kritatestcsplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatestcsplugin_la_LIBADD =  ../../kritacolor/libkritacolor.la 

kritatestcsplugin_la_METASOURCES = AUTO

SUBDIRS = templates
