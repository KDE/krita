# location for the rc file
#kritarcdir = $(kde_datadir)/krita/kpartplugins

# rc file containing the GUI for the plugin
#kritarc_DATA = rgbplugin.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = kritargbplugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

lib_LTLIBRARIES = libkritargb.la
libkritargb_la_SOURCES = kis_rgb_colorspace.cc
libkritargb_la_LDFLAGS = $(all_libraries)
libkritargb_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritargbplugin.la

# Srcs for the plugin
kritargbplugin_la_SOURCES = rgb_plugin.cc
noinst_HEADERS = rgb_plugin.h kis_rgb_colorspace.h

kritargbplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritargbplugin_la_LIBADD =  ../../libkritacommon.la libkritargb.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritargbplugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . templates $(TESTSDIR)

