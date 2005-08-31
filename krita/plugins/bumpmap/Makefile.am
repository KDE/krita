kde_services_DATA = kritabumpmapfilter.desktop

INCLUDES  = -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/widgets \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kritabumpmap_la_SOURCES = bumpmap.cc wdgbumpmap.ui

kde_module_LTLIBRARIES = kritabumpmap.la
noinst_HEADERS = bumpmap.h

kritabumpmap_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritabumpmap_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritabumpmap_la_METASOURCES = AUTO
