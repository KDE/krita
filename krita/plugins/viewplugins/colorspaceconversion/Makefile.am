kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = colorspaceconversion.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)


kde_module_LTLIBRARIES = kritacolorspaceconversion.la

kritacolorspaceconversion_la_SOURCES = wdgconvertcolorspace.ui colorspaceconversion.cc dlg_colorspaceconversion.cc
noinst_HEADERS = wdgconvertcolorspace.h dlg_colorspaceconversion.h colorspaceconversion.h

kritacolorspaceconversion_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacolorspaceconversion_la_LIBADD =  ../../../libkritacommon.la

kritacolorspaceconversion_la_METASOURCES = AUTO

kde_services_DATA = kritacolorspaceconversion.desktop

