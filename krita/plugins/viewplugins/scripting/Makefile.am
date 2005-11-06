kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = scripting.rc

kde_services_DATA = kritascripting.desktop


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$(top_srcdir)/kexi/scriptingcore/main \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritascripting_la_SOURCES = scripting.cc kis_script.cc wdgscriptsmanagerbase.ui \
	wdgscriptsmanager.cpp kis_scripts_registry.cpp

kde_module_LTLIBRARIES = kritascripting.la
noinst_HEADERS = scripting.h kis_script.h wdgscriptsmanager.h

kritascripting_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritascripting_la_LIBADD =  ../../../libkritacommon.la $(top_builddir)/kexi/scriptingcore/main/libkrossmain.la

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

kritascripting_la_METASOURCES = AUTO
