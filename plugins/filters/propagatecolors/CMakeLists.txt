set(kritapropagatecolors_SOURCES
    KisPropagateColorsFilter.cpp
    KisPropagateColorsFilterConfiguration.cpp
    KisPropagateColorsConfigWidget.cpp
    KisPropagateColorsFilterPlugin.cpp
)

kis_add_library(kritapropagatecolors MODULE ${kritapropagatecolors_SOURCES})

target_link_libraries(kritapropagatecolors kritaui)

install(
    TARGETS
    kritapropagatecolors
    DESTINATION 
    ${KRITA_PLUGIN_INSTALL_DIR}
)

install(
    FILES
    propagatecolors.action
    DESTINATION
    ${DATA_INSTALL_DIR}/krita/actions
)
