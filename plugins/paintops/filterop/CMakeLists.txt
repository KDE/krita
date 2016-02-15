set(kritafilterop_SOURCES
    filterop.cpp
    kis_filterop.cpp
    kis_filterop_settings.cpp
    kis_filterop_settings_widget.cpp
    )


add_library(kritafilterop MODULE ${kritafilterop_SOURCES})

target_link_libraries(kritafilterop kritalibpaintop)

install(TARGETS kritafilterop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-filterop.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/images)
