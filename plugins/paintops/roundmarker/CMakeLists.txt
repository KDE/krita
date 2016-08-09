set(kritaroundmarkerpaintop_SOURCES
    roundmarker_paintop_plugin.cpp
    kis_roundmarkerop.cpp
    kis_roundmarkerop_settings.cpp
    kis_roundmarkerop_settings_widget.cpp
)

add_library(kritaroundmarkerpaintop MODULE ${kritaroundmarkerpaintop_SOURCES})

target_link_libraries(kritaroundmarkerpaintop kritalibpaintop)

install(TARGETS kritaroundmarkerpaintop DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  krita-colorsmudge.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

