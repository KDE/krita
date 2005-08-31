KDE_CXXFLAGS = $(USE_EXCEPTIONS)

kde_services_DATA = kritacimg.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/filters \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/dialogs \
	-I$(srcdir)/../../ui/widgets \
	-I../../ui/dialogs \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)

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
kritacimg_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritacimg_la_METASOURCES = AUTO


KDE_OPTIONS = nofinal
