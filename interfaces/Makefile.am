INCLUDES  = $(KOFFICE_INCLUDES) -I$(top_srcdir)/kchart/kdchart -I$(interfacedir) $(all_includes)

lib_LTLIBRARIES = libkochart.la

libkochart_la_SOURCES = koChart.cc
libkochart_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libkochart_la_LIBADD = $(LIB_KOFFICECORE)
METASOURCES = AUTO

include_HEADERS = koChart.h

include $(top_srcdir)/admin/Doxyfile.am
