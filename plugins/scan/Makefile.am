kwordrcdir = $(kde_datadir)/kword/kpartplugins
kwordrc_DATA = scan-kword.rc

krayonrcdir = $(kde_datadir)/krayon/kpartplugins
krayonrc_DATA = scan-krayon.rc

killustratorrcdir = $(kde_datadir)/kontour/kpartplugins
killustratorrc_DATA = scan-kontour.rc

kpresenterrcdir = $(kde_datadir)/kpresenter/kpartplugins
kpresenterrc_DATA = scan-kpresenter.rc

EXTRA_DIST = $(kwordrc_DATA) $(krayonrc_DATA) $(killustratorrc_DATA) $(kpresenterrc_DATA)

INCLUDES = $(KOFFICE_INCLUDES) $(all_includes)
kde_module_LTLIBRARIES = kofficescan.la

kofficescan_la_SOURCES = scan.cpp
noinst_HEADERS = scan.h

kofficescan_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kofficescan_la_LIBADD = $(LIB_KPARTS) $(LIB_KIO) $(LIB_KOFFICECORE)

kofficescan_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kscan_plugin.pot
