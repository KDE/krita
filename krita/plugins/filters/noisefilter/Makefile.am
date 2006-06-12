kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritanoisefilter.desktop


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritanoisefilter_la_SOURCES = noisefilter.cc wdgnoiseoptions.ui \
	kis_wdg_noise.cpp

kde_module_LTLIBRARIES = kritanoisefilter.la
noinst_HEADERS = noisefilter.h kis_wdg_noise.h

kritanoisefilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritanoisefilter_la_LIBADD =  ../../../libkritacommon.la

kritanoisefilter_la_METASOURCES = AUTO
