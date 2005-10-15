kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = variations.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../libkritacolor/ \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../ui \
        -I$(top_srcdir)/lib/kofficecore \
	$(all_includes)

kde_module_LTLIBRARIES = kritavariations.la

kritavariations_la_SOURCES = variations.cc dlg_variations.cc wdg_variations.ui 
noinst_HEADERS = wdg_variations.h

kritavariations_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritavariations_la_LIBADD =  ../../libkritacommon.la

kritavariations_la_METASOURCES = AUTO
