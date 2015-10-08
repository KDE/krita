if (NOT WIN32 AND NOT APPLE)
    add_subdirectory(tests)
endif ()

set(kritadefaulttools_PART_SRCS
    default_tools.cc
    kis_tool_colorpicker.cc
    kis_tool_brush.cc
    kis_tool_line.cc
    kis_tool_line_helper.cpp
    kis_tool_fill.cc
    kis_tool_rectangle.cc
    kis_tool_ellipse.cc
    kis_tool_gradient.cc
    kis_tool_measure.cc
    kis_tool_path.cc
    kis_tool_move.cc
    kis_tool_movetooloptionswidget.cpp
    strokes/move_stroke_strategy.cpp
    strokes/move_selection_stroke_strategy.cpp
    kis_tool_multihand.cpp
    kis_tool_pencil.cc
    )

ki18n_wrap_ui(kritadefaulttools_PART_SRCS wdgcolorpicker.ui wdgmovetool.ui)

add_library(kritadefaulttools MODULE ${kritadefaulttools_PART_SRCS})
generate_export_header(kritadefaulttools BASE_NAME kritadefaulttools)
kcoreaddons_desktop_to_json(kritadefaulttools kritadefaulttools.desktop)

target_link_libraries(kritadefaulttools kritaui kritabasicflakes)
target_link_libraries(kritadefaulttools ${Boost_SYSTEM_LIBRARY})

install(TARGETS kritadefaulttools  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES
# cursors
    tool_rectangle_cursor.png
    tool_ellipse_cursor.png
    tool_line_cursor.png
    tool_freehand_cursor.png
    tool_fill_cursor.png
    tool_gradient_cursor.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

install( FILES
    KisToolPath.action
    KisToolPencil.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
