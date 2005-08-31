kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = perftest.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../paintops \
	-I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritaperftest.la

kritaperftest_la_SOURCES = wdg_perftest.ui perftest.cc dlg_perftest.cc
noinst_HEADERS = wdg_perftest.h dlg_perftest.h perftest.h

kritaperftest_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaperftest_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritaperftest_la_METASOURCES = AUTO
