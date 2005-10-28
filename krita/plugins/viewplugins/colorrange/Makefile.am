INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritacolorrange.la

kritacolorrange_la_SOURCES = colorrange.cc dlg_colorrange.cc wdg_colorrange.ui
noinst_HEADERS = wdg_colorrange.h dlg_colorrange.h colorrange.h

kritacolorrange_la_LIBADD =  ../../../libkritacommon.la
kritacolorrange_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = colorrange.rc
EXTRA_DIST = $(kritarc_DATA)

kde_services_DATA = kritacolorrange.desktop

kritacolorrange_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
