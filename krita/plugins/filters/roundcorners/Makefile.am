kde_services_DATA = kritaroundcornersfilter.desktop

INCLUDES  = \
	 -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritaroundcornersfilter.la

kritaroundcornersfilter_la_SOURCES = kis_round_corners_filter_plugin.cc \
	kis_round_corners_filter.cc

noinst_HEADERS = kis_round_corners_filter_plugin.h \
	kis_round_corners_filter.h

kritaroundcornersfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaroundcornersfilter_la_LIBADD =  ../../../libkritacommon.la

kritaroundcornersfilter_la_METASOURCES = AUTO
