kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = colorspaceconversion.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritacolorspaceconversion.la

kritacolorspaceconversion_la_SOURCES = wdgconvertcolorspace.ui colorspaceconversion.cc dlg_colorspaceconversion.cc
noinst_HEADERS = wdgconvertcolorspace.h dlg_colorspaceconversion.h colorspaceconversion.h

kritacolorspaceconversion_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacolorspaceconversion_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritacolorspaceconversion_la_METASOURCES = AUTO
