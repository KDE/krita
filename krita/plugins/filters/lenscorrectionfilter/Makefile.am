kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritalenscorrectionfilter.desktop


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritalenscorrectionfilter_la_SOURCES = lenscorrectionfilter.cc \
	wdglenscorrectionoptions.ui kis_wdg_lens_correction.cpp

kde_module_LTLIBRARIES = kritalenscorrectionfilter.la
noinst_HEADERS = lenscorrectionfilter.h

kritalenscorrectionfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritalenscorrectionfilter_la_LIBADD =  ../../../libkritacommon.la

METASOURCES = AUTO
