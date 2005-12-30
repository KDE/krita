kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = scripting.rc
EXTRA_DIST = $(kritarc_DATA)

kde_services_DATA = kritascripting.desktop

INCLUDES  =  -I$(top_srcdir)/krita/sdk \
	-I$(top_srcdir)/krita/core \
	-I$(top_srcdir)/krita/kritacolor/ \
	-I$(top_srcdir)/krita/ui \
	$(KROSS_INCLUDES) \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritascripting_la_SOURCES = wdgscriptsmanagerbase.ui scripting.cc kis_script.cc \
	wdgscriptsmanager.cpp kis_scripts_registry.cpp

kde_module_LTLIBRARIES = kritascripting.la
noinst_HEADERS = scripting.h kis_script.h wdgscriptsmanager.h

kritascripting_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritascripting_la_LIBADD =  $(top_builddir)/krita/libkritacommon.la $(top_builddir)/lib/kross/main/libkrossmain.la

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

kritascripting_la_METASOURCES = AUTO
