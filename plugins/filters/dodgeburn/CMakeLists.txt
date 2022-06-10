set(kritadodgeburn_SOURCES DodgeBurnPlugin.cpp DodgeBurn.cpp )
ki18n_wrap_ui(kritadodgeburn_SOURCES
    DodgeBurnConfigurationBaseWidget.ui
    )
kis_add_library(kritadodgeburn MODULE ${kritadodgeburn_SOURCES})
target_link_libraries(kritadodgeburn kritaui)
install(TARGETS kritadodgeburn  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
