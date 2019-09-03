if (NOT APPLE)
    add_subdirectory(tests)
endif ()

set(kritadefaulttools_SOURCES
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
    strokes/move_selection_stroke_strategy.cpp
    KisMoveBoundsCalculationJob.cpp
    kis_tool_multihand.cpp
    kis_tool_multihand_config.cpp
    kis_tool_pencil.cc
    kis_tool_pan.cpp
    )

ki18n_wrap_ui(kritadefaulttools_SOURCES wdgcolorpicker.ui wdgmovetool.ui wdgmultihandtool.ui)
qt5_add_resources(kritadefaulttools_SOURCES defaulttools.qrc )

kis_add_library(kritadefaulttools MODULE ${kritadefaulttools_SOURCES})

generate_export_header(kritadefaulttools BASE_NAME kritadefaulttools)

target_link_libraries(kritadefaulttools kritaui kritabasicflakes)
target_link_libraries(kritadefaulttools ${Boost_SYSTEM_LIBRARY})

install(TARGETS kritadefaulttools  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES
    KisToolPath.action
    KisToolPencil.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
