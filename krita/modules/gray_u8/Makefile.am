#kritarcdir = $(kde_datadir)/krita/kpartplugins
#kritarc_DATA = grayplugin.rc
kde_services_DATA = kritagrayplugin.desktop

#EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	$(all_includes)


lib_LTLIBRARIES = libkritagrayscale.la
libkritagrayscale_la_SOURCES = kis_gray_colorspace.cc
libkritagrayscale_la_LDFLAGS = $(all_libraries)
libkritagrayscale_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kde_module_LTLIBRARIES = kritagrayplugin.la

kritagrayplugin_la_SOURCES = gray_plugin.cc
noinst_HEADERS = gray_plugin.h kis_gray_colorspace.h

kritagrayplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritagrayplugin_la_LIBADD =  ../../libkritacommon.la libkritagrayscale.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritagrayplugin_la_METASOURCES = AUTO

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . templates $(TESTSDIR)

