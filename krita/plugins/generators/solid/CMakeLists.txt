set(kritacolorgenerator_PART_SRCS
    colorgenerator.cpp
    kis_wdg_color.cpp
    )


ki18n_wrap_ui(kritacolorgenerator_PART_SRCS
    wdgcoloroptions.ui
    )

add_library(kritacolorgenerator MODULE ${kritacolorgenerator_PART_SRCS})
kcoreaddons_desktop_to_json(kritacolorgenerator kritacolorgenerator.desktop)

target_link_libraries(kritacolorgenerator kritaui)

install(TARGETS kritacolorgenerator  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacolorgenerator.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
