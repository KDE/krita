########### next target ###############

set(kritawetplugin_PART_SRCS
   kis_wet_colorspace.cc
   wet_plugin.cc
   kis_wetop.cc
   kis_wet_palette_widget.cc
   kis_wetness_visualisation_filter.cc
   kis_texture_painter.cc
   kis_texture_filter.cc
   wetphysicsfilter.cc )

kde4_automoc(${kritawetplugin_PART_SRCS})

kde4_add_ui_files(kritawetplugin_PART_SRCS wdgpressure.ui )

kde4_add_plugin(kritawetplugin ${kritawetplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritawetplugin )

target_link_libraries(kritawetplugin kritaui)

install(TARGETS kritawetplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  wetplugin.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritawetplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

