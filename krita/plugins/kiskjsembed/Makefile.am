kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritakjsembed.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/widgets \
	-I$(srcdir)../../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritakjsembed.la

kritakjsembed_la_SOURCES = kis_kjsembed.cc kis_kjsembed_script.cc
noinst_HEADERS = kis_kjsembed.h kis_kjsembed_script.cc

kritakjsembed_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritakjsembed_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE) -lkjsembed

kritakjsembed_la_METASOURCES = AUTO
