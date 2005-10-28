kde_services_DATA = kritacolorsfilter.desktop

INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritacolorsfilters_la_SOURCES = colorsfilters.cc \
	kis_perchannel_filter.cc \
	wdg_perchannel.ui \
	wdg_brightness_contrast.ui \
	kis_brightness_contrast_filter.cc

noinst_HEADERS = colorsfilters.h \
	kis_perchannel_filter.h \
	kis_brightness_contrast_filter.h

kritacolorsfilters_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacolorsfilters_la_LIBADD =  ../../../libkritacommon.la 

kde_module_LTLIBRARIES = kritacolorsfilters.la

kritacolorsfilters_la_METASOURCES = AUTO
