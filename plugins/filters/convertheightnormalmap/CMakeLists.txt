set(kritaconvertheighttonormalmap_SOURCES
    kis_convert_height_to_normal_map_filter.cpp
    kis_wdg_convert_height_to_normal_map.cpp
)

ki18n_wrap_ui(kritaconvertheighttonormalmap_SOURCES
    wdg_convert_height_to_normal_map.ui
    )
add_library(kritaconvertheighttonormalmap MODULE ${kritaconvertheighttonormalmap_SOURCES})
target_link_libraries(kritaconvertheighttonormalmap kritaui)
install(TARGETS kritaconvertheighttonormalmap  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES
    convertheightnormalmap.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
