kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = substrate.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritasubstrate.la

kritasubstrate_la_SOURCES = substrate.cc dlg_substrate.cc wdgsubstrate.ui kis_repeating_substrate.cc
noinst_HEADERS = wdgsubstrate.h dlg_substrate.h kis_repeating_substrate.h substrate.h

kritasubstrate_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritasubstrate_la_LIBADD =  ../../../libkritacommon.la

kde_services_DATA = kritasubstrate.desktop

METASOURCES = AUTO
