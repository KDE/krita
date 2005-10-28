kde_services_DATA = kritathreadtest.desktop

INCLUDES  =  -I$(srcdir)/../../../sdk \
	 -I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$(srcdir)/../../../../lib/kofficecore \
	$(all_includes)

kritathreadtest_la_SOURCES = threadtest.cc

kde_module_LTLIBRARIES = kritathreadtest.la
noinst_HEADERS = threadtest.h

kritathreadtest_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritathreadtest_la_LIBADD =  ../../../libkritacommon.la

kritathreadtest_la_METASOURCES = AUTO
