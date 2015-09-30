set(kritaimagesplit_PART_SRCS
    imagesplit.cpp
    dlg_imagesplit.cpp
    wdg_imagesplit.cpp
    )

ki18n_wrap_ui(kritaimagesplit_PART_SRCS
    wdg_imagesplit.ui
    )

add_library(kritaimagesplit MODULE ${kritaimagesplit_PART_SRCS})

target_link_libraries(kritaimagesplit kritaui)
kcoreaddons_desktop_to_json(kritaimagesplit kritaimagesplit.desktop)

install(TARGETS kritaimagesplit  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES  imagesplit.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
