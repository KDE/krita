kritarcdir = $(kde_datadir)/krita/kpartplugins

kde_services_DATA = kritaexample.desktop


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../libkritacolor/ \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../ui \
	-I$(interfacedir) \
	-I$(top_srcdir)/lib/kofficecore \
	$(all_includes)

kritaexample_la_SOURCES = example.cc

kde_module_LTLIBRARIES = kritaexample.la
noinst_HEADERS = example.h

kritaexample_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaexample_la_LIBADD =  ../../libkritacommon.la

kritaexample_la_METASOURCES = AUTO
