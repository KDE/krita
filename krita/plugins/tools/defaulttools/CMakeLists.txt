

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

kde4_add_ui_files(kritadefaulttools_PART_SRCS wdgcolorpicker.ui)

kde4_add_plugin(kritadefaulttools ${kritadefaulttools_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritadefaulttools )

target_link_libraries(kritadefaulttools kritaui)

install(TARGETS kritadefaulttools  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_rectangle_cursor.png  	tool_ellipse_cursor.png  	tool_line_cursor.png  	tool_freehand_cursor.png  	tool_duplicate_cursor.png  	tool_fill_cursor.png  	tool_text_cursor.png  	tool_gradient_cursor.png          tool_gradient.png  	tool_line.png          tool_rectangle.png  	tool_ellipse.png          tool_pan.png          tool_freehand.png  	tool_text.png          openhand_cursor.xpm          closedhand_cursor.xpm  	tool_zoom_plus_cursor.png  	tool_zoom_minus_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritadefaulttools.desktop  DESTINATION ${SERVICES_INSTALL_DIR})



