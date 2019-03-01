include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/filterEffectTool
)

########### next target ###############

set(karbon_tools_SOURCES
   KarbonToolsPlugin.cpp
   KarbonCursor.cpp
   CalligraphyTool/KarbonCalligraphyTool.cpp
   CalligraphyTool/KarbonCalligraphyOptionWidget.cpp
   CalligraphyTool/KarbonCalligraphyToolFactory.cpp
   CalligraphyTool/KarbonCalligraphicShape.cpp
   CalligraphyTool/KarbonCalligraphicShapeFactory.cpp
   CalligraphyTool/KarbonSimplifyPath.cpp
   KarbonPatternTool.cpp
   KarbonPatternToolFactory.cpp
   KarbonPatternEditStrategy.cpp
   filterEffectTool/KarbonFilterEffectsTool.cpp
   filterEffectTool/KarbonFilterEffectsToolFactory.cpp
   filterEffectTool/FilterEffectEditWidget.cpp
   filterEffectTool/FilterEffectScene.cpp
   filterEffectTool/FilterEffectSceneItems.cpp
   filterEffectTool/FilterInputChangeCommand.cpp
   filterEffectTool/FilterAddCommand.cpp
   filterEffectTool/FilterRemoveCommand.cpp
   filterEffectTool/FilterStackSetCommand.cpp
   filterEffectTool/FilterRegionChangeCommand.cpp
   filterEffectTool/FilterEffectResource.cpp
   filterEffectTool/FilterResourceServerProvider.cpp
   filterEffectTool/FilterRegionEditStrategy.cpp
   filterEffectTool/KoResourceSelector.cpp
   KarbonPatternOptionsWidget.cpp
)

ki18n_wrap_ui(karbon_tools_SOURCES
    filterEffectTool/FilterEffectEditWidget.ui
    KarbonPatternOptionsWidget.ui
)

qt5_add_resources(karbon_tools_SOURCES karbontools.qrc)

add_library(krita_karbontools MODULE ${karbon_tools_SOURCES})

target_link_libraries(krita_karbontools kritaui kritawidgets KF5::Completion)

install(TARGETS krita_karbontools DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(FILES  CalligraphyTool/KarbonCalligraphyTool.action DESTINATION ${DATA_INSTALL_DIR}/krita/actions)

