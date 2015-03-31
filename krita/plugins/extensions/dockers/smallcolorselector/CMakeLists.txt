
set(KRITA_smallCOLORSELECTOR_PART_SRCS smallcolorselector.cc smallcolorselector_dock.cc kis_small_color_widget.cc )

# ki18n_wrap_ui(kritasmallcolorselector_PART_SRCS myuifile.ui )

add_library(kritasmallcolorselector MODULE ${KRITA_smallCOLORSELECTOR_PART_SRCS})
kcoreaddons_desktop_to_json(kritasmallcolorselector krita_smallcolorselector.desktop)

target_link_libraries(kritasmallcolorselector kritaui)

install(TARGETS kritasmallcolorselector  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_smallcolorselector.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

