kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = shearimage.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../ui \
	-I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritashearimage.la

kritashearimage_la_SOURCES = wdg_shearimage.ui shearimage.cc dlg_shearimage.cc
noinst_HEADERS = wdg_shearimage.h dlg_shearimage.h shearimage.h

kritashearimage_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritashearimage_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritashearimage_la_METASOURCES = AUTO
