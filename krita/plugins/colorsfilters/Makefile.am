kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = colorsfilters.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core -I$(srcdir)/../../core/tiles -I$(srcdir)/../ui -I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = colorsfilters.la

colorsfilters_la_SOURCES = colorsfilters.cc formbcdialog.ui formrgbsliders.ui formcmybsliders.ui
noinst_HEADERS = colorsfilters.h formbcdialog.ui.h formcmybsliders.ui.h


#-Wl --export-dynamic -rdynamic

#CXXFLAGS = -shared -fPIC
colorsfilters_la_LDFLAGS = $(all_libraries) -avoid-version -module
# -shared -Wl
# $(KDE_PLUGIN)
colorsfilters_la_LIBADD =  ../../core/libkiscore.la ../../ui/libkisui.la ../../tools/libkistools.la $(LIB_KPARTS) $(LIB_KIO) $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

colorsfilters_la_METASOURCES = AUTO
