set(kritasketchpaintop_PART_SRCS
    sketch_paintop_plugin.cpp
    kis_sketch_paintop.cpp
    kis_sketchop_option.cpp
    kis_density_option.cpp
    kis_linewidth_option.cpp
    kis_offset_scale_option.cpp
    kis_sketch_paintop_settings.cpp
    kis_sketch_paintop_settings_widget.cpp
    )

kde4_add_ui_files(kritasketchpaintop_PART_SRCS wdgsketchoptions.ui )

kde4_add_plugin(kritasketchpaintop ${kritasketchpaintop_PART_SRCS})

target_link_libraries(kritasketchpaintop kritalibpaintop)

install(TARGETS kritasketchpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasketchpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita-sketch.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


