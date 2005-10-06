kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = shearimage.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../ui \
	-I$(interfacedir) $(all_includes)

kde_module_LTLIBRARIES = kritashearimage.la

kritashearimage_la_SOURCES = wdg_shearimage.ui shearimage.cc dlg_shearimage.cc
noinst_HEADERS = wdg_shearimage.h dlg_shearimage.h shearimage.h

kritashearimage_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritashearimage_la_LIBADD =  ../../libkritacommon.la

kritashearimage_la_METASOURCES = AUTO
