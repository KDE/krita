kde_services_DATA = kritagrayplugin.desktop

INCLUDES  =  -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../kritacolor/ \
	-I$(interfacedir) \
	$(KOFFICE_INCLUDES) \
	$(all_includes)


lib_LTLIBRARIES = libkritagrayscale.la
libkritagrayscale_la_SOURCES = kis_gray_colorspace.cc
libkritagrayscale_la_LDFLAGS = $(all_libraries)
libkritagrayscale_la_LIBADD =  ../../kritacolor/libkritacolor.la

kde_module_LTLIBRARIES = kritagrayplugin.la

kritagrayplugin_la_SOURCES = gray_plugin.cc
noinst_HEADERS = gray_plugin.h kis_gray_colorspace.h

kritagrayplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritagrayplugin_la_LIBADD =  libkritagrayscale.la ../../kritacolor/libkritacolor.la

kritagrayplugin_la_METASOURCES = AUTO

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . templates $(TESTSDIR)

