kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = perftest.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../libkritacolor/ \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../ui \
	-I$(interfacedir) \
        -I$(top_srcdir)/lib/kofficecore \
        -I$(top_srcdir)/lib/kofficeui \
	$(all_includes)

kde_module_LTLIBRARIES = kritaperftest.la

kritaperftest_la_SOURCES = wdg_perftest.ui perftest.cc dlg_perftest.cc
noinst_HEADERS = wdg_perftest.h dlg_perftest.h perftest.h

kritaperftest_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaperftest_la_LIBADD =  ../../libkritacommon.la

kritaperftest_la_METASOURCES = AUTO
