kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = imagesize.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = -I$(srcdir)/../../core -I$(srcdir)/../../core/tiles -I$(srcdir)/../ui -I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = imagesize.la

imagesize_la_SOURCES = wdg_imagesize.ui imagesize.cc dlg_imagesize.cc
noinst_HEADERS = wdg_imagesize.h dlg_imagesize.h imagesize.h


#-Wl --export-dynamic -rdynamic

#CXXFLAGS = -shared -fPIC
imagesize_la_LDFLAGS = $(all_libraries) -avoid-version -module
# -shared -Wl
# $(KDE_PLUGIN)
imagesize_la_LIBADD =  ../../core/libkiscore.la ../../ui/libkisui.la ../../tools/libkistools.la $(LIB_KPARTS) $(LIB_KIO) $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

imagesize_la_METASOURCES = AUTO
