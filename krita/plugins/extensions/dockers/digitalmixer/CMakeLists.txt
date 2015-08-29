
set(KRITA_DIGITALMIXER_PART_SRCS digitalmixer.cc digitalmixer_dock.cc )

# ki18n_wrap_ui(kritadigitalmixer_PART_SRCS myuifile.ui )

add_library(kritadigitalmixer MODULE ${KRITA_DIGITALMIXER_PART_SRCS})
kcoreaddons_desktop_to_json(kritadigitalmixer krita_digitalmixer.desktop)

target_link_libraries(kritadigitalmixer kritaui)

install(TARGETS kritadigitalmixer  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_digitalmixer.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

