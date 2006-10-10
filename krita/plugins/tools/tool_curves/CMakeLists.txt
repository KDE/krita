

set(kritatoolcurves_PART_SRCS
    tool_curves.cc
    kis_curve_framework.cc
    kis_tool_curve.cc
    kis_tool_bezier.cc
    kis_tool_bezier_paint.cc
    kis_tool_bezier_select.cc
    kis_tool_moutline.cc
    kis_tool_example.cc
    )

kde4_automoc(${kritatoolcurves_PART_SRCS})

#kde4_add_ui_files(kritatoolcurves_PART_SRCS wdg_tool_crop.ui)

kde4_add_plugin(kritatoolcurves ${kritatoolcurves_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolcurves )

target_link_libraries(kritatoolcurves kritaui)

install(TARGETS kritatoolcurves  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_bezier_paint.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  tool_bezier_select.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  tool_example_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  tool_moutline_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  tool_moutline.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  tool_curve_dragging.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  tool_example.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  tool_moutline_editing.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  tool_bezier_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolcurves.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
