kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = dropshadow.rc
EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
 	 -I$(srcdir)/../../kritacolor/ \
	 -I$(srcdir)/../../kritacolor/color_strategy \
     -I$(srcdir)/../../ui \
     -I$(top_srcdir)/lib/kofficecore \
     -I$(top_srcdir)/lib/kofficeui \
	 $(all_includes)

kde_module_LTLIBRARIES = kritadropshadow.la

kritadropshadow_la_SOURCES = wdg_dropshadow.ui \
	kis_dropshadow.cc dlg_dropshadow.cc  \
	kis_dropshadow_plugin.cc

noinst_HEADERS = wdg_dropshadow.h kis_dropshadow_plugin.h \
		kis_dropshadow.h dlg_dropshadow.h

kritadropshadow_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritadropshadow_la_LIBADD =  ../../libkritacommon.la 

kritadropshadow_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
