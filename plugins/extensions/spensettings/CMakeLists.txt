set(kritaspensettings_SOURCES
    SPenSettings.cpp
    KisSPenSettings.cpp
    )

ki18n_wrap_ui(kritaspensettings_SOURCES
    wdg_spensettings.ui
    )

qt5_add_resources(kritaspensettings_SOURCES
    spensettings.qrc
)

add_library(kritaspensettings MODULE ${kritaspensettings_SOURCES})
target_link_libraries(kritaspensettings kritaui)
install(TARGETS kritaspensettings  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES  SPenSettings.action  DESTINATION ${DATA_INSTALL_DIR}/krita/actions)

install( FILES  spen-remote.svg
                DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

