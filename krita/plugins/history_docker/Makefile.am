kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritahistorydocker.rc

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/widgets \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kritahistorydocker_la_SOURCES = historydocker.cc kis_docker_history.cc

kde_module_LTLIBRARIES = kritahistorydocker.la
noinst_HEADERS = historydocker.h kis_docker_history.cc

kritahistorydocker_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritahistorydocker_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritahistorydocker_la_METASOURCES = AUTO
