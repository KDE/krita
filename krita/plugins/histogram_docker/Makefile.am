kde_services_DATA = kritahistogramdocker.desktop
kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritahistogramdocker.rc

INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../libkritacolor/ \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../ui \
	-I$(top_srcdir)/lib/kofficecore \
	-I$(top_srcdir)/lib/kopalette \
	$(all_includes)

kritahistogramdocker_la_SOURCES = histogramdocker.cc  kis_imagerasteredcache.cc kis_cachedhistogram.cc kis_accumulating_producer.cc

kde_module_LTLIBRARIES = kritahistogramdocker.la
noinst_HEADERS = histogramdocker.h kis_imagerasteredcache.h kis_cachedhistogram.h kis_accumulating_producer.h

kritahistogramdocker_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritahistogramdocker_la_LIBADD =  ../../libkritacommon.la

kritahistogramdocker_la_METASOURCES = AUTO
