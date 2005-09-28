kde_services_DATA = kritacubismfilter.desktop

INCLUDES  = \
	 -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I../../ui/ \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritacubismfilter.la

kritacubismfilter_la_SOURCES = kis_cubism_filter_plugin.cc \
	kis_cubism_filter.cc \
        kis_polygon.cc

noinst_HEADERS = kis_cubism_filter_plugin.h \
	kis_cubism_filter.h \
        kis_polygon.h

kritacubismfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacubismfilter_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritacubismfilter_la_METASOURCES = AUTO
