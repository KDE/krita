kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = screenshot-krita.rc

EXTRA_DIST =  $(kritarc_DATA) #$(kpresenterrc_DATA) $(kwordrc_DATA)

INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../core/strategy/ $(all_includes)

kde_module_LTLIBRARIES = kritascreenshot.la

kritascreenshot_la_SOURCES = screenshot.cpp ksnapshot.cpp regiongrabber.cpp ksnapshotwidget.ui
noinst_HEADERS = screenshot.h ksnapshot.h regiongrabber.h ksnapshotwidget.h ksnapshotwidget.ui.h

kritascreenshot_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritascreenshot_la_LIBADD =  ../../libkritacommon.la 

kritascreenshot_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kscreenshot_plugin.pot
