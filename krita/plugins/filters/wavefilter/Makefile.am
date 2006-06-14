kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritawavefilter.desktop


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritawavefilter_la_SOURCES = wavefilter.cc wdgwaveoptions.ui \
	kis_wdg_wave.cpp

kde_module_LTLIBRARIES = kritawavefilter.la
noinst_HEADERS = wavefilter.h kis_wdg_wave.h

kritawavefilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritawavefilter_la_LIBADD =  ../../../libkritacommon.la

METASOURCES = AUTO
