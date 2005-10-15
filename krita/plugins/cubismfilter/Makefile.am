kde_services_DATA = kritacubismfilter.desktop

INCLUDES  = \
	 -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../libkritacolor/ \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../ui \
        -I$(top_srcdir)/lib/kofficecore \
        -I$(top_srcdir)/lib/kofficeui \
	$(all_includes)

kde_module_LTLIBRARIES = kritacubismfilter.la

kritacubismfilter_la_SOURCES = kis_cubism_filter_plugin.cc \
	kis_cubism_filter.cc \
        kis_polygon.cc

noinst_HEADERS = kis_cubism_filter_plugin.h \
	kis_cubism_filter.h \
        kis_polygon.h

kritacubismfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacubismfilter_la_LIBADD =  ../../libkritacommon.la

kritacubismfilter_la_METASOURCES = AUTO
