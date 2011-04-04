set(kritaimagesplit_PART_SRCS
    imagesplit.cpp
    dlg_imagesplit.cpp
    wdg_imagesplit.cpp
    )

kde4_add_ui_files(kritaimagesplit_PART_SRCS
    wdg_imagesplit.ui
    )

kde4_add_plugin(kritaimagesplit ${kritaimagesplit_PART_SRCS})

target_link_libraries(kritaimagesplit kritaui)

install(TARGETS kritaimagesplit  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  imagesplit.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaimagesplit.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
