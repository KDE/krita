kde_services_DATA = kritabumpmapfilter.desktop

INCLUDES  = -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../libkritacolor/ \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../ui \
	-I$(interfacedir) \
	-I$(top_srcdir)/lib/kofficecore \
	$(all_includes)

kritabumpmap_la_SOURCES = bumpmap.cc wdgbumpmap.ui

kde_module_LTLIBRARIES = kritabumpmap.la
noinst_HEADERS = bumpmap.h

kritabumpmap_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritabumpmap_la_LIBADD =  ../../libkritacommon.la 

kritabumpmap_la_METASOURCES = AUTO
