
set(KRITA_ANIMATOR_PART_SRCS animator.cpp
    animator_dock.cpp
    kis_layer_contents.cpp
    kis_frame_box.cpp
    kis_animation_frame.cpp
    kis_animation_layerbox.cpp
    kis_animation_layer.cpp
    kis_timeline.cpp
    onionskin_dock.cpp
    kis_opacity_selector.cpp
    kis_opacity_selector_view.cpp
    )

kde4_add_plugin(kritaanimator ${KRITA_ANIMATOR_PART_SRCS})

target_link_libraries(kritaanimator kritaui)

install(TARGETS kritaanimator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_animator.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

