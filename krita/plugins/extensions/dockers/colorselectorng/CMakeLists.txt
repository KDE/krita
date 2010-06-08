
set(KRITA_COLORSELECTORNG_PART_SRCS
        colorselectorng.cpp
        kis_colselng_dock.cpp
        kis_colselng_widget.cpp
        kis_colselng_my_paint_shade_selector.cpp
        kis_colselng_bar.cpp
        kis_colselng_color_patches.cpp
        kis_colselng_color_selector.cpp
        kis_colselng_common_colors.cpp
        kis_colselng_shade_selector.cpp
	kis_colselng_settings.cpp)

kde4_add_plugin(kritacolorselectorng ${KRITA_COLORSELECTORNG_PART_SRCS})

kde4_add_ui_files(KRITA_COLORSELECTORNG_PART_SRCS
    kis_colselng_settings.ui
)

target_link_libraries(kritacolorselectorng kritaui)

install(TARGETS kritacolorselectorng  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############
install( FILES  kis_colselng_color_patches.png
		kis_colselng_color_triangle.png
		kis_colselng_my_paint_shade_selector.png
                DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

install( FILES  krita_colorselectorng.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

