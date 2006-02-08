kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritaimageenhancement.desktop


INCLUDES  = -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritaimageenhancement_la_SOURCES = imageenhancement.cpp \
	kis_simple_noise_reducer.cpp kis_wavelet_noise_reduction.cpp

kde_module_LTLIBRARIES = kritaimageenhancement.la

kritaimageenhancement_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = imageenhancement.h kis_wavelet_noise_reduction.h

kritaimageenhancement_la_LIBADD =  ../../../libkritacommon.la 

kritaimageenhencement_la_METASOURCES = AUTO

KDE_CXXFLAGS = $(USE_EXCEPTIONS)
