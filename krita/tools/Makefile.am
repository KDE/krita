kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritadefaulttools.rc

kde_services_DATA = kritadefaulttools.desktop

EXTRA_DIST = $(kritarc_DATA)


# all_includes must remain last!
INCLUDES = -I$(srcdir)/../core \
	-I$(srcdir)/../core/color_strategy/ \
	-I$(srcdir)/../core/resources/ \
	-I$(srcdir)/../core/tiles \
	-I$(srcdir)/../core/tool \
	-I$(srcdir)/../core/paintop \
	-I$(srcdir)/../ui \
	-I$(srcdir)/../paintops \
	-I$(srcdir)/../ui/dialogs \
	-I$(srcdir)/../ui/widgets \
	-I../ui/dialogs \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../core/strategy/ $(all_includes)

kritadefaulttools_la_SOURCES = \
	default_tools.cc \
	kis_tool_colorpicker.cc \
	kis_tool_move.cc \
	kis_tool_select_rectangular.cc \
	kis_tool_zoom.cc \
	kis_tool_freehand.cc \
	kis_tool_brush.cc \
	kis_tool_eraser.cc \
	kis_tool_line.cc \
	kis_tool_airbrush.cc \
        kis_tool_paste.cc \
        kis_tool_select_freehand.cc \
        kis_tool_select_polygonal.cc \
        kis_tool_select_elliptical.cc \
        kis_tool_select_contiguous.cc \
        kis_tool_pen.cc \
	kis_tool_duplicate.cc \
	kis_tool_filter.cc \
        kis_tool_fill.cc \
        kis_tool_rectangle.cc \
        kis_tool_ellipse.cc \
        kis_tool_polyline.cc \
        kis_tool_polygon.cc \
        kis_tool_colorchanger.cc  \
	kis_tool_select_brush.cc \
	kis_tool_select_eraser.cc \
	kis_tool_pan.cc \
	kis_tool_text.cc \
	kis_tool_gradient.cc

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritadefaulttools.la

noinst_HEADERS = \
	default_tools.h \
	kis_tool_fill.h \
	kis_tool_pen.h \
	kis_tool_select_freehand.h \
	kis_tool_airbrush.h \
	kis_tool_filter.h \
	kis_tool_polygon.h \
	kis_tool_select_polygonal.h \
	kis_tool_brush.h \
	kis_tool_freehand.h \
	kis_tool_polyline.h \
	kis_tool_select_rectangular.h \
	kis_tool_colorchanger.h \
	kis_tool_gradient.h \
	kis_tool_rectangle.h \
	kis_tool_colorpicker.h \
	kis_tool_line.h \
	kis_tool_select_brush.h \
	kis_tool_text.h \
	kis_tool_duplicate.h \
	kis_tool_move.h \
	kis_tool_select_contiguous.h \
	kis_tool_zoom.h \
	kis_tool_ellipse.h \
	kis_tool_pan.h \
	kis_tool_select_elliptical.h \
	kis_tool_eraser.h \
	kis_tool_paste.h \
	kis_tool_select_eraser.h


kritadefaulttools_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritadefaulttools_la_LIBADD =  ../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritadefaulttools_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
