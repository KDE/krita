set(kritaparticlepaintop_SOURCES
    particle_paintop_plugin.cpp
    kis_particle_paintop.cpp
    kis_particleop_option.cpp
    kis_particle_paintop_settings.cpp
    kis_particle_paintop_settings_widget.cpp
    particle_brush.cpp
    )

ki18n_wrap_ui(kritaparticlepaintop_SOURCES wdgparticleoptions.ui )

kis_add_library(kritaparticlepaintop MODULE ${kritaparticlepaintop_SOURCES})

target_link_libraries(kritaparticlepaintop kritalibpaintop)

install(TARGETS kritaparticlepaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita-particle.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


