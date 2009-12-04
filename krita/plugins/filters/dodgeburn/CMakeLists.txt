

set(kritadodgeburn_PART_SRCS DodgeBurnPlugin.cpp DodgeBurn.cpp )

kde4_add_ui_files(kritadodgeburn_PART_SRCS
    DodgeBurnConfigurationBaseWidget.ui
    )

kde4_add_plugin(kritadodgeburn ${kritadodgeburn_PART_SRCS})

target_link_libraries(kritadodgeburn kritaui)

install(TARGETS kritadodgeburn  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadodgeburn.desktop  DESTINATION ${SERVICES_INSTALL_DIR})



