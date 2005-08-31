kde_services_DATA = kritacolorsfilter.desktop

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/filters \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/widgets \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kritacolorsfilters_la_SOURCES = colorsfilters.cc \
	kis_perchannel_filter.cc \
	wdg_brightness_contrast.ui \
	kis_brightness_contrast_filter.cc

noinst_HEADERS = colorsfilters.h \
	kis_perchannel_filter.h \
	kis_brightness_contrast_filter.h

kritacolorsfilters_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacolorsfilters_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kde_module_LTLIBRARIES = kritacolorsfilters.la

kritacolorsfilters_la_METASOURCES = AUTO
