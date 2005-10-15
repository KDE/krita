kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = imagesize.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../kritacolor/ \
	-I$(srcdir)/../../kritacolor/color_strategy \
	-I$(srcdir)/../../ui \
        -I$(top_srcdir)/lib/kofficecore \
        -I$(top_srcdir)/lib/kofficeui \
	-I$(interfacedir) $(all_includes)

kde_module_LTLIBRARIES = kritaimagesize.la

kritaimagesize_la_SOURCES = wdg_imagesize.ui wdg_layersize.ui imagesize.cc dlg_imagesize.cc dlg_layersize.cc
noinst_HEADERS = wdg_imagesize.h dlg_imagesize.h imagesize.h dlg_layersize.h

kritaimagesize_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaimagesize_la_LIBADD =  ../../libkritacommon.la 

kritaimagesize_la_METASOURCES = AUTO
