kde_services_DATA = kritaraindropsfilter.desktop

INCLUDES  = \
	-I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../kritacolor/color_strategy \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../ui \
    $(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritaraindropsfilter.la

kritaraindropsfilter_la_SOURCES = kis_raindrops_filter_plugin.cc \
	kis_raindrops_filter.cc

noinst_HEADERS = kis_raindrops_filter_plugin.h \
	kis_raindrops_filter.h

kritaraindropsfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaraindropsfilter_la_LIBADD =  ../../../libkritacommon.la

kritaraindropsfilter_la_METASOURCES = AUTO
