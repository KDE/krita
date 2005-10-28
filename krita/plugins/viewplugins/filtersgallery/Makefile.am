kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritafiltersgallery.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritafiltersgallery_la_SOURCES = filters_gallery.cc kis_filters_listview.cc \
	kis_dlg_filtersgallery.cc

kde_module_LTLIBRARIES = kritafiltersgallery.la
noinst_HEADERS = filters_gallery.h kis_filters_listview.h kis_dlg_filtersgallery.h

#kde_services_DATA = kritafiltersgallery.desktop

kritafiltersgallery_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritafiltersgallery_la_LIBADD =  ../../../libkritacommon.la 

kritafiltersgallery_la_METASOURCES = AUTO
