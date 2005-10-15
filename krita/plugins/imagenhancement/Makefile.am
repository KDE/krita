kritarcdir = $(kde_datadir)/krita/kpartplugins

kde_services_DATA = kritaimagenhancement.desktop


INCLUDES  = -I$(srcdir)/../../sdk \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../kritacolor/ \
	-I$(srcdir)/../../kritacolor/color_strategy \
    -I$(srcdir)/../../ui \
	-I$(interfacedir) \
    -I$(top_srcdir)/lib/kofficecore \
	$(all_includes)

kritaimagenhancement_la_SOURCES = imagenhancement.cpp \
	kis_simple_noise_reducer.cpp kis_wavelet_noise_reduction.cpp

kde_module_LTLIBRARIES = kritaimagenhancement.la

kritaimagenhancement_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = imagenhancement.h kis_wavelet_noise_reduction.h

kritaimagenhancement_la_LIBADD =  ../../libkritacommon.la 

kritaimagenhencement_la_METASOURCES = AUTO
