kwordrcdir = $(kde_datadir)/kword/kpartplugins
kwordrc_DATA = scan-kword.rc

krayonrcdir = $(kde_datadir)/krayon/kpartplugins
krayonrc_DATA = scan-krayon.rc

killustratorrcdir = $(kde_datadir)/killustrator/kpartplugins
killustratorrc_DATA = scan-killustrator.rc

kpresenterrcdir = $(kde_datadir)/kpresenter/kpartplugins
kpresenterrc_DATA = scan-kpresenter.rc

EXTRA_DIST = $(kwordrc_DATA) $(krayonrc_DATA) $(killustratorrc_DATA) $(kpresenterrc_DATA)

INCLUDES = $(all_includes)
lib_LTLIBRARIES = libkofficescan.la

libkofficescan_la_SOURCES = scan_factory.cpp scan.cpp
noinst_HEADERS = scan_factory.h scan.h

libkofficescan_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkofficescan_la_LIBADD = $(LIB_KPARTS) $(LIB_KIO)

libkofficescan_la_METASOURCES = AUTO

