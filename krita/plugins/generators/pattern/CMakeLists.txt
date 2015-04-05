set(kritapatterngenerator_PART_SRCS
    patterngenerator.cpp
    kis_wdg_pattern.cpp
    )


ki18n_wrap_ui(kritapatterngenerator_PART_SRCS
    wdgpatternoptions.ui
    )

add_library(kritapatterngenerator MODULE ${kritapatterngenerator_PART_SRCS})
kcoreaddons_desktop_to_json(kritapatterngenerator kritapatterngenerator.desktop)

target_link_libraries(kritapatterngenerator kritaui)

install(TARGETS kritapatterngenerator  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritapatterngenerator.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
