set(KRITA_ANIMATIONDOCKER_PART_SRCS
    animation_dockers.cpp
    animation_docker.cpp
    timeline_view.cpp
    timeline_widget.cpp
    timeline_docker.cpp
    keyframe_channel_delegate.cpp
    kis_onion_skin_dialog.cpp
)

kde4_add_ui_files(KRITA_ANIMATIONDOCKER_PART_SRCS
    wdg_animation.ui
    kis_onion_skin_dialog.ui
)

kde4_add_plugin(kritaanimationdocker ${KRITA_ANIMATIONDOCKER_PART_SRCS})

target_link_libraries(kritaanimationdocker kritaui)

install(TARGETS kritaanimationdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_animationdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

