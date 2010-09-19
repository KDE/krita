
set(KRITA_COLORSELECTORNG_PART_SRCS
colorselectorng.cpp
kis_color_selector_ng_dock.cpp
kis_color_selector_ng_docker_widget.cpp
kis_my_paint_shade_selector.cpp
kis_color_patches.cpp
kis_color_history.cpp
kis_color_selector.cpp
kis_common_colors.cpp
kis_common_colors_recalculation_runner.cpp
kis_minimal_shade_selector.cpp
kis_shade_selector_line.cpp
kis_shade_selector_line_combo_box.cpp
kis_shade_selector_lines_settings.cpp
kis_color_selector_settings.cpp
kis_color_selector_container.cpp
kis_color_selector_base.cpp
kis_color_selector_component.cpp
kis_color_selector_ring.cpp
kis_color_selector_triangle.cpp
kis_color_selector_simple.cpp
kis_color_selector_wheel.cpp
kis_color_selector_combo_box.cpp
)

kde4_add_ui_files(KRITA_COLORSELECTORNG_PART_SRCS
    wdg_color_selector_settings.ui
)

kde4_add_plugin(kritacolorselectorng ${KRITA_COLORSELECTORNG_PART_SRCS})

target_link_libraries(kritacolorselectorng kritaui)

install(TARGETS kritacolorselectorng  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############
install( FILES  kis_colselng_color_patches.png
		kis_colselng_color_triangle.png
		kis_colselng_my_paint_shade_selector.png
                extended_color_selector.png
                DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

install( FILES  krita_colorselectorng.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

