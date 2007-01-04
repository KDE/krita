INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritaselectopaque.la

kritaselectopaque_la_SOURCES = selectopaque.cc 
noinst_HEADERS = selectopaque.h

kritaselectopaque_la_LIBADD =  ../../../libkritacommon.la
kritaselectopaque_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = selectopaque.rc
EXTRA_DIST = $(kritarc_DATA)

kde_services_DATA = kritaselectopaque.desktop

kritaselectopaque_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
