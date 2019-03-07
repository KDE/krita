include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/models
)

set(kritasketchplugin_SRCS
    kritasketchplugin.cpp
    ColorSelectorItem.cpp
    CurveEditorItem.cpp
    Constants.cpp
    ToolManager.cpp
    SimpleTouchArea.cpp
    ImageBuilder.cpp
    KritaNamespace.cpp
    MouseTracker.cpp
    PanelConfiguration.cpp
    PropertyContainer.cpp
)

set(kritasketch_kritaintegrationmodels_SRCS
    models/TemplatesModel.cpp
    models/KeyboardModel.cpp
    models/ColorImageProvider.cpp
    models/CompositeOpModel.cpp
    models/FiltersCategoryModel.cpp
    models/FiltersModel.cpp
    models/LayerCompositeDetails.cpp
    models/LayerModel.cpp
    models/LayerThumbProvider.cpp
    models/PaletteColorsModel.cpp
    models/PresetImageProvider.cpp
    models/PresetModel.cpp
    models/RecentImageImageProvider.cpp
    models/RecentImagesModel.cpp
    models/FileSystemModel.cpp
    models/ColorModelModel.cpp
    models/ColorDepthModel.cpp
    models/ColorProfileModel.cpp
    models/IconImageProvider.cpp
)


set(KRITA_COLORSELECTORNG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/plugins/dockers/advancedcolorselector")
set(KRITA_COLORSELECTORNG_SOURCES
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_my_paint_shade_selector.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_patches.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_history.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_common_colors.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_common_colors_recalculation_runner.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_minimal_shade_selector.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_base_proxy.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_shade_selector_line.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_shade_selector_line_editor.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_shade_selector_line_combo_box.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_shade_selector_line_combo_box_popup.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_shade_selector_lines_settings.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_container.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_base.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_component.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_ring.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_triangle.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_simple.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_wheel.cpp
${KRITA_COLORSELECTORNG_SOURCE_DIR}/kis_color_selector_combo_box.cpp
)

if (WIN32)
    add_library(kritasketchplugin MODULE ${kritasketchplugin_SRCS} ${kritasketch_kritaintegrationmodels_SRCS} ${KRITA_COLORSELECTORNG_SOURCES})
else()
    add_library(kritasketchplugin SHARED ${kritasketchplugin_SRCS} ${kritasketch_kritaintegrationmodels_SRCS} ${KRITA_COLORSELECTORNG_SOURCES})
endif()

target_link_libraries(kritasketchplugin
    Qt5::Quick
    Qt5::Gui
    Qt5::Network
    Qt5::Core
    kritawidgets
    kritaui
    kritaqml
)

install(TARGETS   kritasketchplugin DESTINATION ${QML_INSTALL_DIR}/org/krita/sketch)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/krita/sketch)


