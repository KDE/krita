set(kritarulerassistanttool_SOURCES
    Assistant.cc
    EllipseAssistant.cc
    Ellipse.cc
    SplineAssistant.cc
    PerspectiveAssistant.cc
    VanishingPointAssistant.cc
    InfiniteRulerAssistant.cc
    ParallelRulerAssistant.cc
    ConcentricEllipseAssistant.cc
    ruler_assistant_tool.cc
    kis_assistant_tool.cc
    FisheyePointAssistant.cc
    )

ki18n_wrap_ui(kritaassistanttool_SOURCES AssistantsToolOptions.ui )

add_library(kritaassistanttool MODULE ${kritaassistanttool_SOURCES})

target_link_libraries(kritaassistanttool kritaui kritaflake   )

install(TARGETS kritaassistanttool  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES krita_tool_ruler_assistant.png dark_krita_tool_ruler_assistant.png light_krita_tool_ruler_assistant.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
