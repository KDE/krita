kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = rotateimage.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../ui \
	-I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritarotateimage.la

kritarotateimage_la_SOURCES = wdg_rotateimage.ui rotateimage.cc dlg_rotateimage.cc
noinst_HEADERS = wdg_rotateimage.h dlg_rotateimage.h rotateimage.h

kritarotateimage_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritarotateimage_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritarotateimage_la_METASOURCES = AUTO
