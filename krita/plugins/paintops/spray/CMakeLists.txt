set(kritaspraypaintop_PART_SRCS
    spray_paintop_plugin.cpp
    kis_spray_paintop.cpp
    kis_sprayop_option.cpp
    kis_spray_shape_dynamics.cpp
    kis_spray_shape_option.cpp
    kis_spray_paintop_settings.cpp
    kis_spray_paintop_settings_widget.cpp
    spray_brush.cpp
    random_gauss.cpp
    )

ki18n_wrap_ui(kritaspraypaintop_PART_SRCS wdgsprayoptions.ui wdgsprayshapeoptions.ui wdgshapedynamicsoptions.ui )

add_library(kritaspraypaintop MODULE ${kritaspraypaintop_PART_SRCS})
kcoreaddons_desktop_to_json(kritaspraypaintop kritaspraypaintop.desktop)

target_link_libraries(kritaspraypaintop kritalibpaintop)

install(TARGETS kritaspraypaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaspraypaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( FILES
    krita-spray.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

