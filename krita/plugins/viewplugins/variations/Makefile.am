kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = variations.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritavariations.la

kritavariations_la_SOURCES = variations.cc dlg_variations.cc wdg_variations.ui 
noinst_HEADERS = wdg_variations.h

kritavariations_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritavariations_la_LIBADD =  ../../../libkritacommon.la

kde_services_DATA = kritavariations.desktop

kritavariations_la_METASOURCES = AUTO
