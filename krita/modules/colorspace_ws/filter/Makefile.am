kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritawsenginefilter.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = \
	-I$(srcdir)/../../../core/color_strategy \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../core/tiles \
	-I$(srcdir)/../../../core/filters \
	-I$(srcdir)/../../../core/resources \
	-I$(srcdir)/../../../core/tool \
	-I$(srcdir)/../../../core/paintop \
	-I$(srcdir)/../../../ui \
	-I$(srcdir)/../../../ui/widgets \
	-I$(srcdir)/.. \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritawsenginefilter.la

kritawsenginefilter_la_SOURCES = \
	ws_engine_filter_plugin.cc \
	kis_ws_engine_filter.cc

noinst_HEADERS = \
	ws_engine_filter_plugin.h \
        kis_ws_engine_filter.h

kritawsenginefilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritawsenginefilter_la_LIBADD =  ../../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritawsenginefilter_la_METASOURCES = AUTO
