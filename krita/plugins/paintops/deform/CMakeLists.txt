set(kritadeformpaintop_SOURCES
    deform_brush.cpp
    deform_paintop_plugin.cpp
    kis_deform_paintop.cpp
    kis_deform_paintop_settings.cpp
    kis_deform_option.cpp
    kis_deform_paintop_settings_widget.cpp
    )

ki18n_wrap_ui(kritadeformpaintop_SOURCES wdgdeformoptions.ui )

add_library(kritadeformpaintop MODULE ${kritadeformpaintop_SOURCES})

target_link_libraries(kritadeformpaintop kritaui kritalibpaintop)

install(TARGETS kritadeformpaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES  krita-deform.png DESTINATION ${DATA_INSTALL_DIR}/krita/images )
