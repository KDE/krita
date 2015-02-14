set(KRITA_ANIMATIONDOCKER_PART_SRCS
    animation_docker.cpp
    animation_docker_dock.cpp
)

kde4_add_ui_files(KRITA_ANIMATIONDOCKER_PART_SRCS
    wdg_animation.ui
)

kde4_add_plugin(kritaanimationdocker ${KRITA_ANIMATIONDOCKER_PART_SRCS})

target_link_libraries(kritaanimationdocker kritaui)

install(TARGETS kritaanimationdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_animationdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

