set(kritafilterop_PART_SRCS
    filterop.cpp
    kis_filterop.cpp
    kis_filterop_settings.cpp
    kis_filterop_settings_widget.cpp
    )


add_library(kritafilterop MODULE ${kritafilterop_PART_SRCS})

target_link_libraries(kritafilterop kritalibpaintop)

install(TARGETS kritafilterop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritafilterop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

install( FILES
    krita-filterop.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/images)
