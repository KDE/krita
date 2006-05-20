# Install the desktop file needed to detect the plugin
kde_services_DATA = krita_ycbcr_u16_plugin.desktop

INCLUDES  =  -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../kritacolor/ \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

lib_LTLIBRARIES = libkrita_ycbcr_u16.la

libkrita_ycbcr_u16_la_LDFLAGS = $(all_libraries)
libkrita_ycbcr_u16_la_LIBADD =  ../../kritacolor/libkritacolor.la

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = krita_ycbcr_u16_plugin.la

# Srcs for the plugin
krita_ycbcr_u16_plugin_la_SOURCES = ycbcr_u16_plugin.cc
noinst_HEADERS = ycbcr_u16_plugin.h kis_ycbcr_u16_colorspace.h

krita_ycbcr_u16_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_ycbcr_u16_plugin_la_LIBADD =  libkrita_ycbcr_u16.la ../../kritacolor/libkritacolor.la

METASOURCES = AUTO

libkrita_ycbcr_u16_la_SOURCES = kis_ycbcr_u16_colorspace.cc
