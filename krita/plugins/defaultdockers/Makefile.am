kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritadefaultdockers.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../kopalette \
	-I$(srcdir)/../../ui/widgets \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kritadefaultdockers_la_SOURCES = defaultdockers.cc \
	kis_autobrush.cc \
	kis_autogradient.cc \
	kis_birdeye_box.cc \
	kis_brush_chooser.cc \
	kis_gradient_chooser.cc \
	kis_palette_widget.cc \
	kis_pattern_chooser.cc \
	kis_resource_mediator.cc \
	kis_controlframe.cc \
	wdgautobrush.ui \
	wdgautogradient.ui \
	wdgbirdeye.ui

kde_module_LTLIBRARIES = kritadefaultdockers.la

noinst_HEADERS = defaultdockers.h \
	kis_autobrush.h \
	kis_autogradient.h \
	kis_birdeye_box.h \
	kis_brush_chooser.h \
	kis_gradient_chooser.h \
	kis_palette_widget.h \
	kis_pattern_chooser.h \
	kis_controlframe.h \
	kis_resource_mediator.h

kritadefaultdockers_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritadefaultdockers_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritadefaultdockers_la_METASOURCES = AUTO
