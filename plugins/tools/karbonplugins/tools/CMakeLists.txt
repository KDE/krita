set(karbon_tools_SOURCES
   KarbonToolsPlugin.cpp
   CalligraphyTool/KarbonCalligraphyTool.cpp
   CalligraphyTool/KarbonCalligraphyOptionWidget.cpp
   CalligraphyTool/KarbonCalligraphyToolFactory.cpp
   CalligraphyTool/KarbonCalligraphicShape.cpp
   CalligraphyTool/KarbonCalligraphicShapeFactory.cpp
   CalligraphyTool/KarbonSimplifyPath.cpp
)

qt6_add_resources(karbon_tools_SOURCES karbontools.qrc)

kis_add_library(krita_karbontools MODULE ${karbon_tools_SOURCES})

target_link_libraries(krita_karbontools kritaui kritawidgets KF6::Completion)

install(TARGETS krita_karbontools DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(FILES  CalligraphyTool/KarbonCalligraphyTool.action DESTINATION ${DATA_INSTALL_DIR}/krita/actions)

