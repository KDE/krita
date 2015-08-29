set(kritalayersplit_PART_SRCS
    layersplit.cpp
    dlg_layersplit.cpp
    wdg_layersplit.cpp
    )

ki18n_wrap_ui(kritalayersplit_PART_SRCS
    wdg_layersplit.ui
    )

add_library(kritalayersplit MODULE ${kritalayersplit_PART_SRCS})
kcoreaddons_desktop_to_json(kritalayersplit kritalayersplit.desktop)

target_link_libraries(kritalayersplit kritaui)

install(TARGETS kritalayersplit  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES  layersplit.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritalayersplit.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
