set(kritadynapaintop_SOURCES
    dyna_paintop_plugin.cpp
    kis_dyna_paintop.cpp
    kis_dynaop_option.cpp
    kis_dyna_paintop_settings.cpp
    kis_dyna_paintop_settings_widget.cpp
    dyna_brush.cpp
    filter.cpp
    )

ki18n_wrap_ui(kritadynapaintop_SOURCES wdgdynaoptions.ui )

add_library(kritadynapaintop MODULE ${kritadynapaintop_SOURCES})

target_link_libraries(kritadynapaintop kritalibpaintop)

install(TARGETS kritadynapaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-dyna.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


