kde_services_DATA = kritahistogramdocker.desktop
kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritahistogramdocker.rc

INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kritahistogramdocker_la_SOURCES = histogramdocker.cc  kis_imagerasteredcache.cc kis_cachedhistogram.cc kis_accumulating_producer.cc

kde_module_LTLIBRARIES = kritahistogramdocker.la
noinst_HEADERS = histogramdocker.h kis_imagerasteredcache.h kis_cachedhistogram.h kis_accumulating_producer.h

kritahistogramdocker_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritahistogramdocker_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritahistogramdocker_la_METASOURCES = AUTO
