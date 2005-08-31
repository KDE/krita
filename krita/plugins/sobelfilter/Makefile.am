kde_services_DATA = kritasobelfilter.desktop

INCLUDES  = \
	 -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/widgets \
	-I../../ui/widgets \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritasobelfilter.la

kritasobelfilter_la_SOURCES = kis_sobel_filter_plugin.cc \
	kis_sobel_filter.cc

noinst_HEADERS = kis_sobel_filter_plugin.h \
	kis_sobel_filter.h

kritasobelfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritasobelfilter_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritasobelfilter_la_METASOURCES = AUTO
