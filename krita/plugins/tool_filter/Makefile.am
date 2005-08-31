kde_services_DATA = kritatoolfilter.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/filters \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/dialogs \
	-I$(srcdir)/../../ui/widgets \
	-I../../ui/dialogs \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)

kritatoolfilter_la_SOURCES = \
	kis_filterop.cc \
	kis_tool_filter.cc \
	tool_filter.cc

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolfilter.la

noinst_HEADERS = \
	kis_filterop.h \
	kis_tool_filter.h \
	tool_filter.h

kritatoolfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolfilter_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatoolfilter_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
