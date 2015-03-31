set(kritadeformpaintop_PART_SRCS
    deform_brush.cpp
    deform_paintop_plugin.cpp
    kis_deform_paintop.cpp
    kis_deform_paintop_settings.cpp
    kis_deform_option.cpp
    kis_deform_paintop_settings_widget.cpp
    )

ki18n_wrap_ui(kritadeformpaintop_PART_SRCS wdgdeformoptions.ui )

add_library(kritadeformpaintop MODULE ${kritadeformpaintop_PART_SRCS})
kcoreaddons_desktop_to_json(kritadeformpaintop kritadeformpaintop.desktop)

target_link_libraries(kritadeformpaintop kritaui kritalibpaintop)

install(TARGETS kritadeformpaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadeformpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( FILES  krita-deform.png DESTINATION ${DATA_INSTALL_DIR}/krita/images )
