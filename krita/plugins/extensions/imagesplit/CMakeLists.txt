set(kritaimagesplit_SOURCES
    imagesplit.cpp
    dlg_imagesplit.cpp
    wdg_imagesplit.cpp
    )

ki18n_wrap_ui(kritaimagesplit_SOURCES
    wdg_imagesplit.ui
    )

add_library(kritaimagesplit MODULE ${kritaimagesplit_SOURCES})
target_link_libraries(kritaimagesplit kritaui)
install(TARGETS kritaimagesplit  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES  imagesplit.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
