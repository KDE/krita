kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = histogram.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritahistogram.la

kritahistogram_la_SOURCES = histogram.cc dlg_histogram.cc wdghistogram.ui kis_histogram_widget.cc
noinst_HEADERS = dlg_histogram.h histogram.h wdghistogram.h kis_histogram_widget.h

kde_services_DATA = kritahistogram.desktop

kritahistogram_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritahistogram_la_LIBADD =  ../../../libkritacommon.la

kritahistogram_la_METASOURCES = AUTO
