kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = perftest.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritaperftest.la

kritaperftest_la_SOURCES = wdg_perftest.ui perftest.cc dlg_perftest.cc
noinst_HEADERS = wdg_perftest.h dlg_perftest.h perftest.h

kritaperftest_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaperftest_la_LIBADD =  ../../../libkritacommon.la

kde_services_DATA = kritaperftest.desktop

kritaperftest_la_METASOURCES = AUTO
