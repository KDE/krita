kde_services_DATA = kritahistogramdocker.desktop
kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = kritahistogramdocker.rc

INCLUDES  = -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritahistogramdocker_la_SOURCES = histogramdocker.cc  kis_imagerasteredcache.cc kis_cachedhistogram.cc kis_accumulating_producer.cc

kde_module_LTLIBRARIES = kritahistogramdocker.la
noinst_HEADERS = histogramdocker.h kis_imagerasteredcache.h kis_cachedhistogram.h kis_accumulating_producer.h

kritahistogramdocker_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritahistogramdocker_la_LIBADD =  ../../../libkritacommon.la

kritahistogramdocker_la_METASOURCES = AUTO
