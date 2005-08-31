kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = colorrange.rc
EXTRA_DIST = $(kritarc_DATA)

kde_services_DATA = kritatoolselectsimilar.desktop

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)


kritacolorrange_la_SOURCES = colorrange.cc dlg_colorrange.cc wdg_colorrange.ui kis_tool_selectsimilar.cc
noinst_HEADERS = wdg_colorrange.h dlg_colorrange.h colorrange.h kis_tool_selectsimilar.h

kde_module_LTLIBRARIES = kritacolorrange.la

kritacolorrange_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacolorrange_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritacolorrange_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal

# directory for pixmaps
kritapics_DATA=tool_similar_selection.png
kritapicsdir = $(kde_datadir)/krita/pics

