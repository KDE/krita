#kwordrcdir = $(kde_datadir)/kword/kpartplugins
#kwordrc_DATA = screenshot-kword.rc

kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = screenshot-krita.rc

#kpresenterrcdir = $(kde_datadir)/kpresenter/kpartplugins
#kpresenterrc_DATA = screenshot-kpresenter.rc

EXTRA_DIST =  $(kritarc_DATA) #$(kpresenterrc_DATA) $(kwordrc_DATA)

INCLUDES = $(KOFFICE_INCLUDES) $(all_includes)
kde_module_LTLIBRARIES = kritascreenshot.la

kritascreenshot_la_SOURCES = screenshot.cpp ksnapshot.cpp regiongrabber.cpp ksnapshotwidget.ui
noinst_HEADERS = screenshot.h ksnapshot.h regiongrabber.h ksnapshotwidget.h

kritascreenshot_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritascreenshot_la_LIBADD = $(LIB_KPARTS) $(LIB_KIO) $(LIB_KOFFICECORE)

kritascreenshot_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kscreenshot_plugin.pot
