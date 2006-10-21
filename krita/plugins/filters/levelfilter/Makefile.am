kde_services_DATA = kritalevelfilter.desktop

INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritalevelfilter_la_SOURCES = levelfilter.cc \
	wdg_level.ui \
	kis_level_filter.cc \
	kgradientslider.cc

noinst_HEADERS = levelfilter.h \
	kis_level_filter.h \
	kgradientslider.h

kritalevelfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritalevelfilter_la_LIBADD =  ../../../libkritacommon.la 

kde_module_LTLIBRARIES = kritalevelfilter.la

kritalevelfilter_la_METASOURCES = AUTO
