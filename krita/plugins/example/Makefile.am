kritarcdir = $(kde_datadir)/krita/kpartplugins

kde_services_DATA = kritaexample.desktop


INCLUDES  =  -I$(srcdir)/../../sdk \
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

kritaexample_la_SOURCES = example.cc

kde_module_LTLIBRARIES = kritaexample.la
noinst_HEADERS = example.h

kritaexample_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaexample_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritaexample_la_METASOURCES = AUTO
