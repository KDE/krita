kritaimagesdir = $(prefix)/share/apps/krita/images

kritaimages_DATA = \
	airbrush.png \
	paintbrush.png \
	eraser.png \
	pencil.png


kde_services_DATA = kritadefaultpaintops.desktop

# all_includes must remain last!
INCLUDES = -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)


kritadefaultpaintops_la_SOURCES = \
	defaultpaintops_plugin.cc \
	kis_airbrushop.cc \
	kis_brushop.cc \
	kis_duplicateop.cc \
	kis_eraseop.cc \
	kis_penop.cc \
	kis_dlgbrushcurvecontrol.ui

noinst_HEADERS = defaultpaintops_plugin.h 	kis_airbrushop.h 	kis_brushop.h \
		kis_duplicateop.h 	kis_eraseop.h 	kis_penop.h

kde_module_LTLIBRARIES = kritadefaultpaintops.la

kritadefaultpaintops_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritadefaultpaintops_la_LIBADD =  ../../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritadefaultpaintops_la_METASOURCES = AUTO
