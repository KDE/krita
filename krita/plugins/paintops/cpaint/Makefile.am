kde_services_DATA = kritacpaintbrush.desktop

# all_includes must remain last!
INCLUDES = -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)

kritacpaintbrush_la_SOURCES = \
	bristle.cpp \
	brush.cpp \
	gauss.cpp \
	sample.cpp \
	stroke.cpp \
	cpaintbrush.cc \
	kis_cpaintop.cc \
	wdgcpaintoptions.ui

noinst_HEADERS = \
	bristle.h \
	brush.h \
	cpaintbrush.h \
	gauss.h \
	kis_cpaintop.h \
	sample.h \
	stroke.h

kde_module_LTLIBRARIES = kritacpaintbrush.la

kritacpaintbrush_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacpaintbrush_la_LIBADD =  ../../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritacpaintbrush_la_METASOURCES = AUTO
