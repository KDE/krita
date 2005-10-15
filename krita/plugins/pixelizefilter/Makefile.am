kde_services_DATA = kritapixelizefilter.desktop

INCLUDES  = \
	 -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../libkritacolor/ \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../ui \
	-I$(interfacedir) \
        -I$(top_srcdir)/lib/kofficecore \
        -I$(top_srcdir)/lib/kofficeui \
	$(all_includes)

kde_module_LTLIBRARIES = kritapixelizefilter.la

kritapixelizefilter_la_SOURCES = kis_pixelize_filter_plugin.cc \
	kis_pixelize_filter.cc

noinst_HEADERS = kis_pixelize_filter_plugin.h \
	kis_pixelize_filter.h

kritapixelizefilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritapixelizefilter_la_LIBADD =  ../../libkritacommon.la

kritapixelizefilter_la_METASOURCES = AUTO
