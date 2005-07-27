kde_services_DATA = kritaseparatechannels.desktop

INCLUDES  = \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/widgets \
	-I../../ui/widgets \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritaseparatechannels.la

kritaseparatechannels_la_SOURCES = kis_separate_channels_plugin.cc \
	kis_separate_channels.cc

noinst_HEADERS = kis_separate_channels_plugin.h \
	kis_separate_channels.h

kritaseparatechannels_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaseparatechannels_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritaseparatechannels_la_METASOURCES = AUTO
