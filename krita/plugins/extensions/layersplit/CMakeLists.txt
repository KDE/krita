set(kritalayersplit_PART_SRCS
    layersplit.cpp
    dlg_layersplit.cpp
    wdg_layersplit.cpp
    )

kde4_add_ui_files(kritalayersplit_PART_SRCS
    wdg_layersplit.ui
    )

kde4_add_plugin(kritalayersplit ${kritalayersplit_PART_SRCS})

target_link_libraries(kritalayersplit kritaui)

install(TARGETS kritalayersplit  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  layersplit.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritalayersplit.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
