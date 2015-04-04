

set(kritadodgeburn_PART_SRCS DodgeBurnPlugin.cpp DodgeBurn.cpp )

ki18n_wrap_ui(kritadodgeburn_PART_SRCS
    DodgeBurnConfigurationBaseWidget.ui
    )

add_library(kritadodgeburn MODULE ${kritadodgeburn_PART_SRCS})
kcoreaddons_desktop_to_json(kritadodgeburn kritadodgeburn.desktop)

target_link_libraries(kritadodgeburn kritaui)

install(TARGETS kritadodgeburn  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadodgeburn.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)



