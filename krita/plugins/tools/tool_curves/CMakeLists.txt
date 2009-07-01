add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)


set(kritatoolcurves_PART_SRCS
#    tool_curves.cc
    kis_curve_framework.cc
    kis_curve_magnetic.cpp
    kis_tool_curve.cc
#    kis_tool_bezier.cc
#    kis_tool_bezier_paint.cc
#    kis_tool_bezier_select.cc
#    kis_tool_moutline.cc
#    kis_tool_example.cc
    kis_curve_framework2.cpp
    )

kde4_add_plugin(kritatoolcurves ${kritatoolcurves_PART_SRCS})


target_link_libraries(kritatoolcurves kritaui 
    ${KDE4_KDE3SUPPORT_LIBS}
)

install(TARGETS kritatoolcurves  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_bezier_paint.png tool_bezier_select.png tool_example_cursor.png tool_moutline_cursor.png tool_moutline.png tool_curve_dragging.png tool_example.png tool_moutline_editing.png tool_bezier_cursor.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

install( FILES  kritatoolcurves.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
