if (NOT WIN32)
    add_subdirectory(tests)
endif (NOT WIN32)

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
    kis_tool_path.cc
    kis_tool_move.cc
    strokes/move_stroke_strategy.cpp
    kis_tool_multihand.cpp
    )

kde4_add_ui_files(kritadefaulttools_PART_SRCS wdgcolorpicker.ui wdgmovetool.ui)

kde4_add_plugin(kritadefaulttools ${kritadefaulttools_PART_SRCS})

target_link_libraries(kritadefaulttools kritaui)
if (HAVE_OPENGL)
  target_link_libraries(kritadefaulttools ${QT_QTOPENGL_LIBRARY} ${OPENGL_LIBRARIES} )
endif (HAVE_OPENGL)
if (HAVE_GLEW)
  target_link_libraries(kritadefaulttools ${GLEW_LIBRARIES} )
endif (HAVE_GLEW)

install(TARGETS kritadefaulttools  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES
    krita_tool_color_fill.png
    krita_tool_color_picker.png
    krita_tool_move.png
    krita_tool_gradient.png
    krita_tool_line.png
    krita_tool_rectangle.png
    krita_tool_ellipse.png
    krita_tool_freehand.png
    krita_tool_multihand.png
    krita_tool_measure.png
    krita_draw_path.png
# cursors
    tool_rectangle_cursor.png
    tool_ellipse_cursor.png
    tool_line_cursor.png
    tool_freehand_cursor.png
    tool_fill_cursor.png
    tool_gradient_cursor.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

install( FILES  kritadefaulttools.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

