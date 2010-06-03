
set(KRITA_COLORSELECTORNG_PART_SRCS colorselectorng.cpp kis_colselng_dock.cpp kis_colselng_widget.cpp kis_colselng_my_paint_shade_selector.cpp )

kde4_add_plugin(kritacolorselectorng ${KRITA_COLORSELECTORNG_PART_SRCS})

target_link_libraries(kritacolorselectorng kritaui)

install(TARGETS kritacolorselectorng  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_colorselectorng.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

