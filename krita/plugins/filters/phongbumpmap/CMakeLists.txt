set(kritaphongbumpmap_PART_SRCS
    kis_phong_bumpmap_plugin.cpp
    kis_phong_bumpmap_config_widget.cpp
    kis_phong_bumpmap_filter.cpp
    phong_pixel_processor.cpp
    )

kde4_add_ui_files(kritaphongbumpmap_PART_SRCS wdgphongbumpmap.ui)

kde4_add_plugin(kritaphongbumpmap ${kritaphongbumpmap_PART_SRCS})

target_link_libraries(kritaphongbumpmap kritaui)

install(TARGETS kritaphongbumpmap  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaphongbumpmapfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

