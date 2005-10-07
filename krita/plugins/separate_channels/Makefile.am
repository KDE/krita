kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = imageseparate.rc
EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
        -I$(srcdir)/../../core/color_strategy \
        -I$(srcdir)/../../ui \
        -I$(interfacedir) \
        -I$(top_srcdir)/lib/kofficecore \
	 $(all_includes)

kde_module_LTLIBRARIES = kritaseparatechannels.la

kritaseparatechannels_la_SOURCES = wdg_separations.ui \
	kis_channel_separator.cc dlg_separate.cc  \
	kis_separate_channels_plugin.cc

noinst_HEADERS = wdg_separations.h kis_separate_channels_plugin.h \
		kis_channel_separator.h dlg_separate.h

kritaseparatechannels_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaseparatechannels_la_LIBADD =  ../../libkritacommon.la 

kritaseparatechannels_la_METASOURCES = AUTO
