kde4_header()

set(kritadefaulttools_PART_SRCS
    default_tools.cc
    kis_tool_colorpicker.cc
    kis_tool_move.cc
    kis_tool_zoom.cc
    kis_tool_brush.cc
    kis_tool_line.cc
    kis_tool_duplicate.cc
    kis_tool_fill.cc
    kis_tool_rectangle.cc
    kis_tool_ellipse.cc
    kis_tool_pan.cc
    kis_tool_text.cc
    kis_tool_gradient.cc
    )

kde4_automoc(${kritadefaulttools_PART_SRCS})

kde4_add_ui3_files(kritadefaulttools_PART_SRCS wdgcolorpicker.ui)

kde4_add_plugin(kritadefaulttools ${kritadefaulttools_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritadefaulttools )

target_link_libraries(kritadefaulttools  ${KDE4_KDECORE_LIBS} kritacommon kopainter kofficecore )

install_targets(${PLUGIN_INSTALL_DIR} kritadefaulttools )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/pics FILES  tool_rectangle_cursor.png  	tool_ellipse_cursor.png  	tool_line_cursor.png  	tool_freehand_cursor.png  	tool_duplicate_cursor.png  	tool_fill_cursor.png  	tool_text_cursor.png  	tool_gradient_cursor.png          tool_gradient.png  	tool_line.png          tool_rectangle.png  	tool_ellipse.png          tool_pan.png          tool_freehand.png  	tool_text.png          openhand_cursor.xpm          closedhand_cursor.xpm  	tool_zoom_plus_cursor.png  	tool_zoom_minus_cursor.png )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritadefaulttools.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritadefaulttools.desktop
#
## all_includes must remain last!
#INCLUDES = -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritadefaulttools_la_SOURCES = \
#	default_tools.cc \
#	kis_tool_colorpicker.cc \
#	kis_tool_move.cc \
#	kis_tool_zoom.cc \
#	kis_tool_brush.cc \
#	kis_tool_line.cc \
#	kis_tool_duplicate.cc \
#    kis_tool_fill.cc \
#    kis_tool_rectangle.cc \
#    kis_tool_ellipse.cc \
#	kis_tool_pan.cc \
#	kis_tool_text.cc \
#	kis_tool_gradient.cc \
#	wdgcolorpicker.ui
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritadefaulttools.la
#
#noinst_HEADERS = \
#	default_tools.h \
#	kis_tool_fill.h \
#	kis_tool_brush.h \
#	kis_tool_gradient.h \
#	kis_tool_rectangle.h \
#	kis_tool_colorpicker.h \
#	kis_tool_line.h \
#	kis_tool_text.h \
#	kis_tool_duplicate.h \
#	kis_tool_move.h \
#	kis_tool_zoom.h \
#	kis_tool_ellipse.h \
#	kis_tool_pan.h 
#
#
#kritadefaulttools_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritadefaulttools_la_LIBADD =  ../../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)
#
#kritadefaulttools_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
#kritapics_DATA = \
#	tool_rectangle_cursor.png \
#	tool_ellipse_cursor.png \
#	tool_line_cursor.png \
#	tool_freehand_cursor.png \
#	tool_duplicate_cursor.png \
#	tool_fill_cursor.png \
#	tool_text_cursor.png \
#	tool_gradient_cursor.png \
#        tool_gradient.png \
#	tool_line.png \
#        tool_rectangle.png \
#	tool_ellipse.png \
#        tool_pan.png \
#        tool_freehand.png \
#	tool_text.png \
#        openhand_cursor.xpm \
#        closedhand_cursor.xpm \
#	tool_zoom_plus_cursor.png \
#	tool_zoom_minus_cursor.png
#
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
