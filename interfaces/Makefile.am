INCLUDES  = $(KOFFICE_INCLUDES) -I$(top_srcdir)/kchart/kdchart -I$(interfacedir) $(all_includes)

lib_LTLIBRARIES = libkochart.la

libkochart_la_SOURCES = koChart.cc
libkochart_la_LDFLAGS = $(all_libraries) -version-info 1:0:0
libkochart_la_LIBADD = $(top_builddir)/lib/kofficecore/libkofficecore.la
METASOURCES = AUTO

include_HEADERS = koChart.h
