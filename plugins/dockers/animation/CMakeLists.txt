add_subdirectory(tests)

set(KRITA_ANIMATIONDOCKER_SOURCES
    KisAnimUtils.cpp
    
    KisTimeBasedItemModel.cpp

    KisAnimationPlaybackControlsModel.cpp
    KisAnimTimelineDocker.cpp
    KisAnimTimelineFramesModel.cpp
    KisAnimTimelineFramesView.cpp
    KisAnimTimelineFrameDelegate.cpp
    KisAnimTimelineLayersHeader.cpp
    KisAnimTimelineTimeHeader.cpp
    KisAnimTimelineColors.cpp

    KisAnimCurvesDocker.cpp
    KisAnimCurvesModel.cpp
    KisAnimCurvesView.cpp
    KisAnimCurvesKeyDelegate.cpp
    KisAnimCurvesChannelsModel.cpp
    KisAnimCurvesChannelDelegate.cpp
    KisAnimCurvesValuesHeader.cpp

    KisOnionSkinsDocker.cpp

    kis_equalizer_column.cpp
    kis_equalizer_slider.cpp
    kis_equalizer_button.cpp
    kis_equalizer_widget.cpp

    timeline_insert_keyframe_dialog.cpp
    timeline_frames_index_converter.cpp
    timeline_node_list_keeper.cpp

    kis_draggable_tool_button.cpp
    kis_zoom_button.cpp
    kis_custom_modifiers_catcher.cpp
)

ki18n_wrap_ui(KRITA_ANIMATIONDOCKER_SOURCES
    KisOnionSkinsDocker.ui
)

kis_add_library(kritaanimationdocker_static STATIC ${KRITA_ANIMATIONDOCKER_SOURCES})

generate_export_header(kritaanimationdocker_static BASE_NAME kritaanimationdocker EXPORT_MACRO_NAME KRITAANIMATIONDOCKER_EXPORT)

target_link_libraries(kritaanimationdocker_static kritaui kritawidgets)

kis_add_library(kritaanimationdocker MODULE KisAnimDockers.cpp)

target_link_libraries(kritaanimationdocker kritaanimationdocker_static)

install(TARGETS kritaanimationdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
