set(kritaphongbumpmap_PART_SRCS
    kis_phong_bumpmap_plugin.cpp
    kis_phong_bumpmap_config_widget.cpp
    kis_phong_bumpmap_filter.cpp
    phong_pixel_processor.cpp
    )

ki18n_wrap_ui(kritaphongbumpmap_PART_SRCS wdgphongbumpmap.ui)

add_library(kritaphongbumpmap MODULE ${kritaphongbumpmap_PART_SRCS})
kcoreaddons_desktop_to_json(kritaphongbumpmap kritaphongbumpmapfilter.desktop)

target_link_libraries(kritaphongbumpmap kritaui)

install(TARGETS kritaphongbumpmap  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaphongbumpmapfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

