# all_includes must remain last!
INCLUDES = -I$(srcdir)/../core \
	-I$(srcdir)/../core/color_strategy/ \
	-I$(srcdir)/../core/compositeop \
	-I$(srcdir)/../core/tiles \
	-I$(srcdir)/../ui \
	-I$(srcdir)/../ui/dialogs \
	-I../ui/dialogs \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../core/strategy/ $(all_includes)

noinst_LTLIBRARIES = libkistools.la

libkistools_la_SOURCES = kis_tool_paint.cc \
	kis_tool_non_paint.cc \
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

libkistools_la_METASOURCES = AUTO



