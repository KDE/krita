
set(KRITA_DIGITALMIXER_SOURCES digitalmixer.cc digitalmixer_dock.cc )

# ki18n_wrap_ui(kritadigitalmixer_SOURCES myuifile.ui )

add_library(kritadigitalmixer MODULE ${KRITA_DIGITALMIXER_SOURCES})
kcoreaddons_desktop_to_json(kritadigitalmixer krita_digitalmixer.desktop)

target_link_libraries(kritadigitalmixer kritaui)

install(TARGETS kritadigitalmixer  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
