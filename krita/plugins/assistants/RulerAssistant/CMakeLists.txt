include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RulerAssistantCommon)

set(kritarulerassistanttool_PART_SRCS
    RulerAssistant.cc
    EllipseAssistant.cc
    Ellipse.cc
    SplineAssistant.cc
    PerspectiveAssistant.cc
    VanishingPointAssistant.cc
    InfiniteRulerAssistant.cc
    ParallelRulerAssistant.cc
    ConcentricEllipseAssistant.cc
    ruler_assistant_tool.cc
    kis_ruler_assistant_tool.cc
    FisheyePointAssistant.cc
    )

ki18n_wrap_ui(kritarulerassistanttool_PART_SRCS AssistantsToolOptions.ui )

add_library(kritarulerassistanttool MODULE ${kritarulerassistanttool_PART_SRCS})
kcoreaddons_desktop_to_json(kritarulerassistanttool kritarulerassistanttool.desktop)

target_link_libraries(kritarulerassistanttool kritaui flake   )

install(TARGETS kritarulerassistanttool  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES krita_tool_ruler_assistant.png dark_krita_tool_ruler_assistant.png light_krita_tool_ruler_assistant.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritarulerassistanttool.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
