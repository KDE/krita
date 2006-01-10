kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = imagesize.rc
EXTRA_DIST = $(kritarc_DATA)

kde_services_DATA = kritaimagesize.desktop


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritaimagesize_la_SOURCES = wdg_imagesize.ui wdg_layersize.ui imagesize.cc dlg_imagesize.cc dlg_layersize.cc wdg_resolution.ui

noinst_HEADERS = wdg_imagesize.h dlg_imagesize.h imagesize.h dlg_layersize.h

kde_module_LTLIBRARIES = kritaimagesize.la

kritaimagesize_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaimagesize_la_LIBADD =  ../../../libkritacommon.la 

kritaimagesize_la_METASOURCES = AUTO
