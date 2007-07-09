

set(kritafilterop_PART_SRCS
    filterop.cc
    kis_filterop.cc
    )

kde4_automoc(${kritafilterop_PART_SRCS})

kde4_add_ui_files(kritafilterop_PART_SRCS FilterOpOptionsWidget.ui)

kde4_add_plugin(kritafilterop ${kritafilterop_PART_SRCS})



target_link_libraries(kritafilterop kritaui)

install(TARGETS kritafilterop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritafilterop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

install( FILES
    filterop.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/images)
