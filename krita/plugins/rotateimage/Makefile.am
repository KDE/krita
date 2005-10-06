kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = rotateimage.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../ui \
	-I$(interfacedir) \
	$(all_includes)

kde_module_LTLIBRARIES = kritarotateimage.la

kritarotateimage_la_SOURCES = wdg_rotateimage.ui rotateimage.cc dlg_rotateimage.cc
noinst_HEADERS = wdg_rotateimage.h dlg_rotateimage.h rotateimage.h

kritarotateimage_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritarotateimage_la_LIBADD =  ../../libkritacommon.la

kritarotateimage_la_METASOURCES = AUTO
