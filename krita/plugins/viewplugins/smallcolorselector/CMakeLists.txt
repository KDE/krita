
set(KRITA_smallCOLORSELECTOR_PART_SRCS smallcolorselector.cc smallcolorselector_dock.cc kis_small_color_widget.cc )

# kde4_add_ui_files(kritasmallcolorselector_PART_SRCS myuifile.ui )

kde4_add_plugin(kritasmallcolorselector ${KRITA_smallCOLORSELECTOR_PART_SRCS})

target_link_libraries(kritasmallcolorselector kritaui)

install(TARGETS kritasmallcolorselector  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_smallcolorselector.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

