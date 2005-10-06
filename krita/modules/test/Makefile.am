# location for the rc file
#kritarcdir = $(kde_datadir)/krita/kpartplugins

# rc file containing the GUI for the plugin
#kritarc_DATA = testcsplugin.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = kritatestcsplugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatestcsplugin.la

# Srcs for the plugin
kritatestcsplugin_la_SOURCES = testcs_plugin.cc kis_test_colorspace.cc
noinst_HEADERS = testcs_plugin.h kis_test_colorspace.h

kritatestcsplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatestcsplugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatestcsplugin_la_METASOURCES = AUTO
#METASOURCES = AUTO # XXX: which of the two?

SUBDIRS = templates
