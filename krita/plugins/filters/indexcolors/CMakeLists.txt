set(INDEXCOLORS_SOURCE kiswdgindexcolors.cpp indexcolors.cpp indexcolorpalette.cpp palettegeneratorconfig.cpp)

kde4_add_ui_files(INDEXCOLORS_SOURCE kiswdgindexcolors.ui)

kde4_add_plugin(kritaindexcolors ${INDEXCOLORS_SOURCE})
target_link_libraries(kritaindexcolors kritaui)

install(TARGETS kritaindexcolors          DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   kritaindexcolors.desktop  DESTINATION ${SERVICES_INSTALL_DIR})