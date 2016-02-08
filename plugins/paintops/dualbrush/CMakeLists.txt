set(kritadualbrushpaintop_SOURCES
    DualBrush.cpp
    DualBrushOption.cpp
    DualBrushPaintop.cpp
    DualBrushPaintopPlugin.cpp
    DualBrushPaintopSettings.cpp
    DualBrushPaintopSettingsWidget.cpp
)

ki18n_wrap_ui(kritadualbrushpaintop_SOURCES WdgDualBrushOptions.ui )

add_library(kritadualbrushpaintop MODULE ${kritadualbrushpaintop_SOURCES})

target_link_libraries(kritadualbrushpaintop kritalibpaintop)

install(TARGETS kritadualbrushpaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-dualbrush.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
