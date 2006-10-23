

set(kritabumpmap_PART_SRCS
    bumpmap.cc
    )

kde4_automoc(${kritabumpmap_PART_SRCS})

kde4_add_ui_files(kritabumpmap_PART_SRCS wdgbumpmap.ui)

kde4_add_plugin(kritabumpmap ${kritabumpmap_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritabumpmap )

target_link_libraries(kritabumpmap kritaui)

install(TARGETS kritabumpmap  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritabumpmapfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

