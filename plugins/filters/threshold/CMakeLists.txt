set(kritathreshold_SOURCES threshold.cpp )

ki18n_wrap_ui(kritathreshold_SOURCES
    wdg_threshold.ui
    )

kis_add_library(kritathreshold MODULE ${kritathreshold_SOURCES})
target_link_libraries(kritathreshold kritaui)
install(TARGETS kritathreshold  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES
    threshold.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)

