kritaimagesdir = $(prefix)/share/apps/krita/images

kritaimages_DATA = \
	airbrush.png \
	paintbrush.png \
	eraser.png \
	pencil.png


kde_services_DATA = kritasmearybrush.desktop

# all_includes must remain last!
INCLUDES = -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/color_strategy/ \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)


kritasmearybrush_la_SOURCES = \
	smearybrush.cc \
	kis_smearyop.cc \

noinst_HEADERS = smearybrush.h 	kis_smearyop.h

kde_module_LTLIBRARIES = kritasmearybrush.la

kritasmearybrush_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritasmearybrush_la_LIBADD =  ../../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritasmearybrush_la_METASOURCES = AUTO
