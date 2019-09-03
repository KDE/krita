set(kritalayersplit_SOURCES
    layersplit.cpp
    dlg_layersplit.cpp
    wdg_layersplit.cpp
    )

ki18n_wrap_ui(kritalayersplit_SOURCES
    wdg_layersplit.ui
    )

kis_add_library(kritalayersplit MODULE ${kritalayersplit_SOURCES})
target_link_libraries(kritalayersplit kritaui)
install(TARGETS kritalayersplit  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  layersplit.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
