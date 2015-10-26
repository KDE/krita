set(KRITA_ANIMATIONDOCKER_PART_SRCS
    animation_dockers.cpp
    animation_docker.cpp
    timeline_view.cpp
    timeline_widget.cpp
    timeline_docker.cpp
    keyframe_channel_delegate.cpp
    kis_onion_skin_dialog.cpp
)

ki18n_wrap_ui(KRITA_ANIMATIONDOCKER_PART_SRCS
    wdg_animation.ui
    kis_onion_skin_dialog.ui
)

add_library(kritaanimationdocker MODULE ${KRITA_ANIMATIONDOCKER_PART_SRCS})

kcoreaddons_desktop_to_json(kritaanimationdocker krita_animationdocker.desktop)

target_link_libraries(kritaanimationdocker kritaui)

install(TARGETS kritaanimationdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


