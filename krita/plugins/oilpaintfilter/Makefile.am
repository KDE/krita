kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritaoilpaintfilter.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = \
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

kde_module_LTLIBRARIES = kritaoilpaintfilter.la

kritaoilpaintfilter_la_SOURCES = kis_oilpaint_filter_configuration_base_widget.ui \
	kis_oilpaint_filter_plugin.cc \
        kis_oilpaint_filter_configuration_widget.cc \
	kis_oilpaint_filter.cc

noinst_HEADERS = kis_oilpaint_filter_configuration_widget.h \
	kis_oilpaint_filter_plugin.h \
        kis_oilpaint_filter.h

kritaoilpaintfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaoilpaintfilter_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritaoilpaintfilter_la_METASOURCES = AUTO
