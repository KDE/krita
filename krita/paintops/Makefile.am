# all_includes must remain last!
INCLUDES = -I$(srcdir)/../core \
	-I$(srcdir)/../core/color_strategy/ \
	-I$(srcdir)/../core/resources/ \
	-I$(srcdir)/../core/tiles \
	-I$(srcdir)/../core/paintop \
	-I$(srcdir)/../ui \
	-I$(srcdir)/../ui/dialogs \
	-I$(srcdir)/../ui/widgets \
	-I../ui/dialogs \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../core/strategy/ $(all_includes)

noinst_LTLIBRARIES = libkispaintops.la

libkispaintops_la_SOURCES = \
	kis_airbrushop.cc \
	kis_brushop.cc \
	kis_convolveop.cc \
	kis_duplicateop.cc \
	kis_eraseop.cc \
	kis_filterop.cc \
	kis_penop.cc

noinst_HEADERS= \
	kis_airbrushop.h \
	kis_brushop.h \
	kis_convolveop.h \
	kis_duplicateop.h \
	kis_eraseop.h \
	kis_filterop.h \
	kis_penop.h

libkispaintops_la_METASOURCES = AUTO
