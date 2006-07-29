kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritafastcolortransfer.desktop


INCLUDES  = -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritafastcolortransfer_la_SOURCES = wdgfastcolortransfer.ui fastcolortransfer.cc \
	kis_wdg_fastcolortransfer.cpp

kde_module_LTLIBRARIES = kritafastcolortransfer.la
noinst_HEADERS = fastcolortransfer.h kis_wdg_fastcolortransfer.h

kritafastcolortransfer_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritafastcolortransfer_la_LIBADD = ../../../libkritacommon.la

METASOURCES = AUTO
