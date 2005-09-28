kritarcdir = $(kde_datadir)/krita/kpartplugins

kde_services_DATA = kritaimagenhancement.desktop


INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kritaimagenhancement_la_SOURCES = imagenhancement.cpp \
        kis_simple_noise_reducer.cpp

kde_module_LTLIBRARIES = kritaimagenhancement.la

kritaimagenhancement_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = imagenhancement.h

kritaimagenhancement_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritaimagenhencement_la_METASOURCES = AUTO
