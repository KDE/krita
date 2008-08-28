
set(KRITA_SPECIFICCOLORSELECTOR_PART_SRCS specificcolorselector.cc specificcolorselector_dock.cc kis_specific_color_selector_widget.cc kis_color_input.cc )

# kde4_add_ui_files(kritaspecificcolorselector_PART_SRCS myuifile.ui )

kde4_add_plugin(kritaspecificcolorselector ${KRITA_SPECIFICCOLORSELECTOR_PART_SRCS})

target_link_libraries(kritaspecificcolorselector kritaui ${OPENEXR_LIBRARIES})

install(TARGETS kritaspecificcolorselector  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_specificcolorselector.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

