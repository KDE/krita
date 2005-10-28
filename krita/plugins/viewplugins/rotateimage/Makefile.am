kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = rotateimage.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritarotateimage.la

kritarotateimage_la_SOURCES = wdg_rotateimage.ui rotateimage.cc dlg_rotateimage.cc
noinst_HEADERS = wdg_rotateimage.h dlg_rotateimage.h rotateimage.h

kritarotateimage_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritarotateimage_la_LIBADD =  ../../../libkritacommon.la

kde_services_DATA = kritarotateimage.desktop

kritarotateimage_la_METASOURCES = AUTO
