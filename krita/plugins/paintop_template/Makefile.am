kde_services_DATA = kritatemplatepaintops.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../kritacolor/ \
	-I$(srcdir)/../../kritacolor/color_strategy \
	-I$(srcdir)/../../ui \
	$(all_includes)


kritatemplatepaintops_la_SOURCES = \
	templatepaintops_plugin.cc \
	kis_templateop.cc

noinst_HEADERS= \
	templatepaintops_plugin.h \
	kis_templateop.h

kde_module_LTLIBRARIES = kritatemplatepaintops.la

kritatemplatepaintops_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatemplatepaintops_la_LIBADD =  ../../libkritacommon.la

kritatemplatepaintops_la_METASOURCES = AUTO
