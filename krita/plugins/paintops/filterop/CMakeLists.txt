set(kritafilterop_PART_SRCS
    filterop.cpp
    kis_filterop.cpp
    kis_filterop_settings.cpp
    kis_filterop_settings_widget.cpp
    )


add_library(kritafilterop MODULE ${kritafilterop_PART_SRCS})
kcoreaddons_desktop_to_json(kritafilterop kritafilterop.desktop)

target_link_libraries(kritafilterop kritalibpaintop)

install(TARGETS kritafilterop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-filterop.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/images)
