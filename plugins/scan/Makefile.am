kwordrcdir = $(kde_datadir)/kword/kpartplugins
kwordrc_DATA = scan-kword.rc

kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = scan-krita.rc

kpresenterrcdir = $(kde_datadir)/kpresenter/kpartplugins
kpresenterrc_DATA = scan-kpresenter.rc

EXTRA_DIST = $(kwordrc_DATA) $(kritarc_DATA) $(kpresenterrc_DATA)

INCLUDES = $(KOFFICE_INCLUDES) $(all_includes)
kde_module_LTLIBRARIES = kofficescan.la

kofficescan_la_SOURCES = scan.cpp
noinst_HEADERS = scan.h

kofficescan_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kofficescan_la_LIBADD = $(LIB_KPARTS) $(LIB_KIO) $(LIB_KOFFICECORE)

kofficescan_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kscan_plugin.pot
