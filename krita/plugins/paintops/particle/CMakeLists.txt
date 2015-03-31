set(kritaparticlepaintop_PART_SRCS
    particle_paintop_plugin.cpp
    kis_particle_paintop.cpp
    kis_particleop_option.cpp
    kis_particle_paintop_settings.cpp
    kis_particle_paintop_settings_widget.cpp
    particle_brush.cpp
    )

ki18n_wrap_ui(kritaparticlepaintop_PART_SRCS wdgparticleoptions.ui )

add_library(kritaparticlepaintop MODULE ${kritaparticlepaintop_PART_SRCS})
kcoreaddons_desktop_to_json(kritaparticlepaintop kritaparticlepaintop.desktop)

target_link_libraries(kritaparticlepaintop kritalibpaintop)

install(TARGETS kritaparticlepaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaparticlepaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( FILES  krita-particle.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


