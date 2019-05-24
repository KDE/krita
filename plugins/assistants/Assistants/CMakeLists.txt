set(kritaassistanttool_SOURCES
    assistant_tool.cc
    ConcentricEllipseAssistant.cc
    Ellipse.cc
    EllipseAssistant.cc
    FisheyePointAssistant.cc
    InfiniteRulerAssistant.cc
    kis_assistant_tool.cc
    ParallelRulerAssistant.cc
    PerspectiveAssistant.cc
    Ruler.cc
    RulerAssistant.cc
    SplineAssistant.cc
    VanishingPointAssistant.cc
    AddRemoveAssistantCommand.cpp
)

ki18n_wrap_ui(kritaassistanttool_SOURCES AssistantsToolOptions.ui )

add_library(kritaassistanttool MODULE ${kritaassistanttool_SOURCES})

target_link_libraries(kritaassistanttool kritaui kritaflake   )

install(TARGETS kritaassistanttool  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES krita_tool_assistant.png dark_krita_tool_assistant.png light_krita_tool_assistant.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
