kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = colorrange.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core -I$(srcdir)/../../core/tiles -I$(srcdir)/../../ui -I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = colorrange.la

colorrange_la_SOURCES = colorrange.cc dlg_colorrange.cc wdg_colorrange.ui 
noinst_HEADERS = wdg_colorrange.h dlg_colorrange.h colorrange.h


#-Wl --export-dynamic -rdynamic

#CXXFLAGS = -shared -fPIC
colorrange_la_LDFLAGS = $(all_libraries) -avoid-version -module
# -shared -Wl
# $(KDE_PLUGIN)
colorrange_la_LIBADD =  ../../core/libkiscore.la ../../ui/libkisui.la ../../tools/libkistools.la $(LIB_KPARTS) $(LIB_KIO) $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

colorrange_la_METASOURCES = AUTO
