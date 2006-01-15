SUBDIRS = kritascripting kritacore samples

kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = scripting.rc

kde_services_DATA = kritascripting.desktop


INCLUDES  =  -I$(top_srcdir)/krita/sdk \
	-I$(top_srcdir)/krita/core \
	-I$(top_srcdir)/krita/kritacolor/ \
	-I$(top_srcdir)/krita/ui \
        -I$(top_builddir)/lib/kross/main \
	$(KROSS_INCLUDES) \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritascripting_la_SOURCES = scripting.cc

kde_module_LTLIBRARIES = kritascripting.la
noinst_HEADERS = scripting.h

kritascripting_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritascripting_la_LIBADD =  $(top_builddir)/krita/libkritacommon.la $(top_builddir)/lib/kross/main/libkrossmain.la ./kritascripting/libkritascripting.la

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

kritascripting_la_METASOURCES = AUTO
