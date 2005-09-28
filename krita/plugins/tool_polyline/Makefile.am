kde_services_DATA = kritatoolpolyline.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../tools \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)

kritatoolpolyline_la_SOURCES = \
        tool_polyline.cc \
	kis_tool_polyline.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolpolyline.la

noinst_HEADERS = \
	tool_polyline.h \
	kis_tool_polyline.h 

kritatoolpolyline_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolpolyline_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatoolpolyline_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
