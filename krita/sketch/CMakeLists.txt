if (WIN32)
    option(ENABLE_SKETCH_STARTUP_MAGIC "Enable Sketch's windows startup magic" ON)

    if (ENABLE_SKETCH_STARTUP_MAGIC)
        add_definitions(-DSKETCH_ENABLE_STARTUP_MAGIC)
    endif ()
endif ()

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(SYSTEM
    ${Boost_INCLUDE_DIRS}
)

add_subdirectory(draganddrop)

set(kritasketchlib_SRCS
    DocumentManager.cpp
    DocumentListModel.cpp
    KisSelectionExtras.cpp
    KisSketchView.cpp
    RecentFileManager.cpp
    ProgressProxy.cpp
    PropertyContainer.cpp
    Settings.cpp
    SketchDeclarativeView.cpp
#     SketchInputContext.cpp
    VirtualKeyboardController.cpp
    Theme.cpp
    QmlGlobalEngine.cpp
)

set(kritasketch_SRCS
    SketchApplication.cpp
    main.cpp
    MainWindow.cpp
)
qt5_add_resources(kritasketch_SRCS  ${krita_QRCS})

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
    models/PaletteModel.cpp
    models/PresetImageProvider.cpp
    models/PresetModel.cpp
    models/RecentImageImageProvider.cpp
    models/RecentImagesModel.cpp
    models/FileSystemModel.cpp
    models/ColorModelModel.cpp
    models/ColorDepthModel.cpp
    models/ColorProfileModel.cpp
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
    set(kritasketch_SRCS ${kritasketch_SRCS} KritaSketchWin.rc)
else ()
    file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icon/*-apps-kritasketch.png")
    ecm_add_app_icon(kritasketch_SRCS ICONS ${ICONS_SRCS})
endif ()

# Krita Sketch library with shared things needed in both plugin and application
# =============================================================================

add_library(kritasketchlib SHARED ${kritasketchlib_SRCS})
generate_export_header(kritasketchlib BASE_NAME krita_sketch)
set_target_properties(kritasketchlib
    PROPERTIES VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)


target_link_libraries(kritasketchlib
    Qt5::Quick
    Qt5::Gui
    Qt5::Core
    kritawidgets
    kritaui
)

include(GNUInstallDirs)

# install(TARGETS kritasketchlib DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS kritasketchlib ${INSTALL_TARGETS_DEFAULT_ARGS})


# Krita Sketch application
# =============================================================================

add_executable(kritasketch ${kritasketch_SRCS})

target_link_libraries(kritasketch
    Qt5::Quick
    Qt5::Gui
    Qt5::Core
    kritaui
    kritasketchlib
)

install(TARGETS   kritasketch ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS  kritasketch.desktop               DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES     kritasketchrc kritasketchpanelsrc DESTINATION ${CONFIG_INSTALL_DIR} )
install(FILES     kritasketch.qml                   DESTINATION ${DATA_INSTALL_DIR}/kritasketch)
install(FILES     kritasketch.xmlgui                    DESTINATION ${DATA_INSTALL_DIR}/kritasketch)

# Sketch Qt Quick components
# =============================================================================

add_library(kritasketchplugin SHARED ${kritasketchplugin_SRCS} ${kritasketch_kritaintegrationmodels_SRCS} ${KRITA_COLORSELECTORNG_SOURCES})

target_link_libraries(kritasketchplugin
    Qt5::Quick
    Qt5::Gui
    Qt5::Network
    Qt5::Core
    kritawidgets
    kritaui
    kritasketchlib
)

install(TARGETS   kritasketchplugin DESTINATION ${QML_INSTALL_DIR}/org/krita/sketch)
install(FILES
                 qmldir
                 qml/HelpPage.qml
                 qml/KritaSketchBase.qml
                 qml/MainPage.qml
                 qml/OpenImagePage.qml
                 qml/SaveImagePage.qml
                 qml/SettingsPage.qml
                 qml/WelcomePage.qml
                 qml/CustomImagePage.qml
                                    DESTINATION ${QML_INSTALL_DIR}/org/krita/sketch)
install(DIRECTORY qml/components    DESTINATION ${QML_INSTALL_DIR}/org/krita/sketch PATTERN "*~" EXCLUDE)
install(DIRECTORY qml/images        DESTINATION ${QML_INSTALL_DIR}/org/krita/sketch PATTERN "*~" EXCLUDE)
install(DIRECTORY qml/panels        DESTINATION ${QML_INSTALL_DIR}/org/krita/sketch PATTERN "*~" EXCLUDE)

install(DIRECTORY themes DESTINATION ${DATA_INSTALL_DIR}/kritasketch)

add_subdirectory(icon)
