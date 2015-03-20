add_subdirectory( tests )

set(kritalayerstylesfilter_PART_SRCS
    layerstyles.cpp
    kis_ls_drop_shadow_filter.cpp
    kis_ls_satin_filter.cpp
    kis_ls_utils.cpp
    kis_layer_style_filter.cpp
    kis_layer_style_filter_projection_plane.cpp
    kis_layer_style_projection_plane.cpp
    )

# kde4_add_ui_files(kritalayerstylesfilter_PART_SRCS
#  wdglayerstyles.ui
# )

kde4_add_plugin(kritalayerstylesfilter ${kritalayerstylesfilter_PART_SRCS})



target_link_libraries(kritalayerstylesfilter kritaui libkispsd)

install(TARGETS kritalayerstylesfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritalayerstylesfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
