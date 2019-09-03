set(kritahalftone_SOURCES kis_halftone_filter.cpp )

ki18n_wrap_ui(kritahalftone_SOURCES
    wdg_halftone_filter.ui
    )

kis_add_library(kritahalftone MODULE ${kritahalftone_SOURCES})
target_link_libraries(kritahalftone kritaui)
install(TARGETS kritahalftone  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

