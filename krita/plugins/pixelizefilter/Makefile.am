kde_services_DATA = kritapixelizefilter.desktop

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

kde_module_LTLIBRARIES = kritapixelizefilter.la

kritapixelizefilter_la_SOURCES = kis_pixelize_filter_plugin.cc \
	kis_pixelize_filter.cc

noinst_HEADERS = kis_pixelize_filter_plugin.h \
	kis_pixelize_filter.h

kritapixelizefilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritapixelizefilter_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritapixelizefilter_la_METASOURCES = AUTO
