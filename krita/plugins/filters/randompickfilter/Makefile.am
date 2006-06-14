kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritarandompickfilter.desktop


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritarandompickfilter_la_SOURCES = randompickfilter.cc wdgrandompickoptions.ui \
	kis_wdg_random_pick.cpp

kde_module_LTLIBRARIES = kritarandompickfilter.la
noinst_HEADERS = randompickfilter.h

kritarandompickfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritarandompickfilter_la_LIBADD =  ../../../libkritacommon.la

kritarandompickfilter_la_METASOURCES = AUTO
