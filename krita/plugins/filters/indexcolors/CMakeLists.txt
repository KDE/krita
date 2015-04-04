set(INDEXCOLORS_SOURCE kiswdgindexcolors.cpp indexcolors.cpp indexcolorpalette.cpp palettegeneratorconfig.cpp)

ki18n_wrap_ui(INDEXCOLORS_SOURCE kiswdgindexcolors.ui)

add_library(kritaindexcolors MODULE ${INDEXCOLORS_SOURCE})
kcoreaddons_desktop_to_json(kritaindexcolors kritaindexcolors.desktop)
target_link_libraries(kritaindexcolors kritaui)

install(TARGETS kritaindexcolors          DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install(FILES   kritaindexcolors.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)