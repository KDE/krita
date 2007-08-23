set(kritadefaulttools_PART_SRCS
    default_tools.cc
    kis_tool_colorpicker.cc
    kis_tool_brush.cc
    kis_tool_line.cc
    kis_tool_fill.cc
    kis_tool_rectangle.cc
    kis_tool_ellipse.cc
    kis_tool_gradient.cc
    kis_tool_measure.cc
    )

kde4_add_ui_files(kritadefaulttools_PART_SRCS wdgcolorpicker.ui)

kde4_add_plugin(kritadefaulttools ${kritadefaulttools_PART_SRCS})

target_link_libraries(kritadefaulttools kritaui)

install(TARGETS kritadefaulttools  DESTINATION ${PLUGIN_INSTALL_DIR})
