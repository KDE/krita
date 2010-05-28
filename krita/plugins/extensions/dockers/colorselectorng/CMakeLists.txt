
set(KRITA_COLORSELECTORNG_PART_SRCS colorselectorng.cc colorselectorng_dock.cc colorselectorngwidget.cc )

kde4_add_plugin(kritacolorselectorng ${KRITA_COLORSELECTORNG_PART_SRCS})

target_link_libraries(kritacolorselectorng kritaui)

install(TARGETS kritacolorselectorng  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_colorselectorng.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

