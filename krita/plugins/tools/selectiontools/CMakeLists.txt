

set(kritaselectiontools_PART_SRCS
    selection_tools.cc
    kis_tool_select_rectangular.cc
    kis_tool_select_brush.cc
    kis_tool_select_eraser.cc
    kis_tool_select_polygonal.cc
    kis_tool_select_elliptical.cc
    kis_tool_select_contiguous.cc
    kis_tool_select_outline.cc
    )

kde4_automoc(${kritaselectiontools_PART_SRCS})

kde4_add_plugin(kritaselectiontools ${kritaselectiontools_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaselectiontools )

target_link_libraries(kritaselectiontools kritaui)

install(TARGETS kritaselectiontools  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_rect_selection.png  	tool_eraser_selection.png  	tool_brush_selection.png   	tool_contiguous_selection.png  	tool_elliptical_selection.png  	tool_outline_selection.png  	tool_polygonal_selection.png  	tool_rectangular_selection_cursor.png  	tool_eraser_selection_cursor.png  	tool_brush_selection_cursor.png   	tool_contiguous_selection_cursor.png  	tool_elliptical_selection_cursor.png  	tool_outline_selection_cursor.png  	tool_polygonal_selection_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritaselectiontools.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


