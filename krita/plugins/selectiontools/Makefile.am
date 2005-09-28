kde_services_DATA = kritaselectiontools.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I../../ui \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)

kritaselectiontools_la_SOURCES = \
	selection_tools.cc \
	kis_tool_select_rectangular.cc \
	kis_tool_select_brush.cc \
	kis_tool_select_eraser.cc \
        kis_tool_select_polygonal.cc \
        kis_tool_select_elliptical.cc \
        kis_tool_select_contiguous.cc \
        kis_tool_select_outline.cc

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritaselectiontools.la

noinst_HEADERS = \
	selection_tools.h \
	kis_tool_select_outline.h \
	kis_tool_select_polygonal.h \
	kis_tool_select_rectangular.h \
	kis_tool_select_brush.h \
	kis_tool_select_eraser.h \
	kis_tool_select_contiguous.h \
	kis_tool_select_elliptical.h 


kritaselectiontools_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaselectiontools_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritaselectiontools_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal

kritapics_DATA = \
	tool_rect_selection.png \
	tool_eraser_selection.png \
	tool_brush_selection.png  \
	tool_contiguous_selection.png \
	tool_elliptical_selection.png \
	tool_outline_selection.png \
	tool_polygonal_selection.png

kritapicsdir = $(kde_datadir)/krita/pics

