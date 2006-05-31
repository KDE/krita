KDE_CXXFLAGS = $(USE_EXCEPTIONS) -D_GNU_SOURCE

kde_services_DATA = kritacimg.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritacimg_la_SOURCES = \
	wdg_cimg.ui\
	kis_cimg_filter.cc\
	kis_cimg_plugin.cc\
	kis_cimgconfig_widget.cc

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritacimg.la

noinst_HEADERS = \
	CImg.h\
	kis_cimg_filter.h\
	kis_cimg_plugin.h\
	kis_cimgconfig_widget.h

kritacimg_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacimg_la_LIBADD =  ../../../libkritacommon.la

kritacimg_la_METASOURCES = AUTO


KDE_OPTIONS = nofinal
