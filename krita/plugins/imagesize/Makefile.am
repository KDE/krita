kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = imagesize.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../ui \
	-I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = imagesize.la

imagesize_la_SOURCES = wdg_imagesize.ui imagesize.cc dlg_imagesize.cc
noinst_HEADERS = wdg_imagesize.h dlg_imagesize.h imagesize.h

imagesize_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
imagesize_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

imagesize_la_METASOURCES = AUTO
