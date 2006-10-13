

set(kritadynamicbrush_PART_SRCS
    dynamicbrush.cc
    kis_dynamicop.cc
    kis_dynamic_transformation.cc
    kis_size_transformation.cc
    kis_darken_transformation.cc
    )

kde4_automoc(kritadynamicbrush ${kritadynamicbrush_PART_SRCS})

kde4_add_plugin(kritadynamicbrush ${kritadynamicbrush_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritadynamicbrush )

target_link_libraries(kritadynamicbrush kritaui)

install(TARGETS kritadynamicbrush  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadynamicbrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

