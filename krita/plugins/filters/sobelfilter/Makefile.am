kde_services_DATA = kritasobelfilter.desktop

INCLUDES  = \
	 -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritasobelfilter.la

kritasobelfilter_la_SOURCES = kis_sobel_filter_plugin.cc \
	kis_sobel_filter.cc

noinst_HEADERS = kis_sobel_filter_plugin.h \
	kis_sobel_filter.h

kritasobelfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritasobelfilter_la_LIBADD =  ../../../libkritacommon.la

kritasobelfilter_la_METASOURCES = AUTO
