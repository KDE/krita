set(kritaroundmarkerpaintop_SOURCES
    roundmarker_paintop_plugin.cpp
    kis_roundmarkerop.cpp
    kis_roundmarkerop_settings.cpp
    kis_roundmarkerop_settings_widget.cpp
    kis_roundmarker_option.cpp
)

ki18n_wrap_ui(kritaroundmarkerpaintop_SOURCES kis_roundmarker_option.ui )

kis_add_library(kritaroundmarkerpaintop MODULE ${kritaroundmarkerpaintop_SOURCES})
target_link_libraries(kritaroundmarkerpaintop kritalibpaintop)


install(TARGETS kritaroundmarkerpaintop DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  krita_roundmarkerop.svg DESTINATION ${DATA_INSTALL_DIR}/krita/images)

