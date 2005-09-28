kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritafiltersgallery.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../ui \
	-I../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kritafiltersgallery_la_SOURCES = filters_gallery.cc kis_filters_listview.cc \
	kis_dlg_filtersgallery.cc

kde_module_LTLIBRARIES = kritafiltersgallery.la
noinst_HEADERS = filters_gallery.h kis_filters_listview.h kis_dlg_filtersgallery.h

kritafiltersgallery_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritafiltersgallery_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritafiltersgallery_la_METASOURCES = AUTO
