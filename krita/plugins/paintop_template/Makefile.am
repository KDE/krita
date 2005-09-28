kde_services_DATA = kritatemplatepaintops.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)


kritatemplatepaintops_la_SOURCES = \
	templatepaintops_plugin.cc \
	kis_templateop.cc

noinst_HEADERS= \
	templatepaintops_plugin.h \
	kis_templateop.h

kde_module_LTLIBRARIES = kritatemplatepaintops.la

kritatemplatepaintops_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatemplatepaintops_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatemplatepaintops_la_METASOURCES = AUTO
