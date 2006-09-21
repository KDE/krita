kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritablurfilter.desktop

INCLUDES = -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritablurfilter_la_SOURCES = wdgblur.ui blur.cc kis_blur_filter.cc kis_wdg_blur.cc

kde_module_LTLIBRARIES = kritablurfilter.la
noinst_HEADERS = blur.h kis_blur_filter.h

kritablurfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritablurfilter_la_LIBADD = ../../../libkritacommon.la

METASOURCES = AUTO

