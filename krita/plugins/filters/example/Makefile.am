kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritaexample.desktop


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritaexample_la_SOURCES = example.cc

kde_module_LTLIBRARIES = kritaexample.la
noinst_HEADERS = example.h

kritaexample_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaexample_la_LIBADD =  ../../../libkritacommon.la

kritaexample_la_METASOURCES = AUTO
