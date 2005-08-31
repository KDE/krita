kde_services_DATA = kritatoolpolygon.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/dialogs \
	-I$(srcdir)/../../ui/widgets \
	-I$(srcdir)/../../tools \
        -I../../ui/dialogs \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)

kritatoolpolygon_la_SOURCES = \
        tool_polygon.cc \
	kis_tool_polygon.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolpolygon.la

noinst_HEADERS = \
	tool_polygon.h \
	kis_tool_polygon.h 

kritatoolpolygon_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolpolygon_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatoolpolygon_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
