set(kritaspraypaintop_SOURCES
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

ki18n_wrap_ui(kritaspraypaintop_SOURCES wdgsprayoptions.ui wdgsprayshapeoptions.ui wdgshapedynamicsoptions.ui )

add_library(kritaspraypaintop MODULE ${kritaspraypaintop_SOURCES})

target_link_libraries(kritaspraypaintop kritalibpaintop)

install(TARGETS kritaspraypaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES
    krita-spray.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

