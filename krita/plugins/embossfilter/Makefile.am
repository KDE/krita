kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritaembossfilter.rc

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

kde_module_LTLIBRARIES = kritaembossfilter.la

kritaembossfilter_la_SOURCES = kis_emboss_filter_plugin.cc \
	kis_emboss_filter.cc

noinst_HEADERS = kis_emboss_filter_plugin.h \
        kis_emboss_filter.h

kritaembossfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaembossfilter_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritaembossfilter_la_METASOURCES = AUTO
