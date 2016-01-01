add_subdirectory(tests)

set(KRITA_ANIMATIONDOCKER_SOURCES
    animation_dockers.cpp
    animation_docker.cpp
    timeline_docker.cpp
    onion_skins_docker.cpp

    timeline_layers_header.cpp
    timeline_ruler_header.cpp

    timeline_frames_model.cpp
    timeline_frames_view.cpp
    timeline_frames_item_delegate.cpp

    timeline_frames_index_converter.cpp
    timeline_node_list_keeper.cpp

    timeline_color_scheme.cpp

    kis_draggable_tool_button.cpp
    kis_animation_utils.cpp
    kis_custom_modifiers_catcher.cpp

    kis_equalizer_column.cpp
    kis_equalizer_slider.cpp
    kis_equalizer_button.cpp
    kis_equalizer_widget.cpp
)

ki18n_wrap_ui(KRITA_ANIMATIONDOCKER_SOURCES
    wdg_animation.ui
    onion_skins_docker.ui
)

add_library(kritaanimationdocker MODULE ${KRITA_ANIMATIONDOCKER_SOURCES})
generate_export_header(kritaanimationdocker BASE_NAME kritaanimationdocker EXPORT_MACRO_NAME KRITAANIMATIONDOCKER_EXPORT)
target_link_libraries(kritaanimationdocker kritaui)
install(TARGETS kritaanimationdocker  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


