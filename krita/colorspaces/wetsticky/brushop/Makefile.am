kde_services_DATA = kritawsbrushpaintop.desktop

kritaimagesdir = $(prefix)/share/apps/krita/images
kritaimages_DATA = wetpaintbrush.png

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../../color_strategy/ \
	-I$(srcdir)/../../../core/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)


kritawsbrushpaintop_la_SOURCES = \
	wsbrushpaintop_plugin.cc \
	kis_wsbrushop.cc

noinst_HEADERS= \
	wsbrushpaintop_plugin.h \
	kis_wsbrushop.h

kde_module_LTLIBRARIES = kritawsbrushpaintop.la

kritawsbrushpaintop_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritawsbrushpaintop_la_LIBADD =  ../../..//libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritawsbrushpaintop_la_METASOURCES = AUTO
