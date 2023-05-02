add_subdirectory(tests)

set(kritaassistanttool_SOURCES
    ConcentricEllipseAssistant.cc
    CurvilinearPerspectiveAssistant.cc
    Ellipse.cc
    EllipseAssistant.cc
    FisheyePointAssistant.cc
    InfiniteRulerAssistant.cc
    kis_assistant_tool.cc
    ParallelRulerAssistant.cc
    PerspectiveBasedAssistantHelper.cpp
    PerspectiveAssistant.cc
    Ruler.cc
    RulerAssistant.cc
    SplineAssistant.cc
    VanishingPointAssistant.cc
    EditAssistantsCommand.cpp
    TwoPointAssistant.cc
    PerspectiveEllipseAssistant.cc
    EllipseInPolygon.cpp
)

ki18n_wrap_ui(kritaassistanttool_SOURCES AssistantsToolOptions.ui )

kis_add_library(kritaassistanttool_static STATIC ${kritaassistanttool_SOURCES})

generate_export_header(kritaassistanttool_static BASE_NAME kritaassistanttool EXPORT_MACRO_NAME KRITAASSISTANTTOOL_EXPORT)

target_link_libraries(kritaassistanttool_static kritaui kritaflake gsl)

kis_add_library(kritaassistanttool MODULE assistant_tool.cc)
target_link_libraries(kritaassistanttool kritaassistanttool_static)

install(TARGETS kritaassistanttool  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES krita_tool_assistant.png dark_krita_tool_assistant.png light_krita_tool_assistant.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
