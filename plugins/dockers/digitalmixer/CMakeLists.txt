set(KRITA_DIGITALMIXER_SOURCES digitalmixer.cc digitalmixer_dock.cc )
add_library(kritadigitalmixer MODULE ${KRITA_DIGITALMIXER_SOURCES})
target_link_libraries(kritadigitalmixer kritaui)
install(TARGETS kritadigitalmixer  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
