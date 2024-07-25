set(kritaedgedetection_SOURCES
    kis_edge_detection_filter.cpp
    kis_wdg_edge_detection.cpp
)

ki18n_wrap_ui(kritaedgedetection_SOURCES
    wdg_edge_detection.ui
    )
kis_add_library(kritaedgedetection MODULE ${kritaedgedetection_SOURCES})
target_link_libraries(kritaedgedetection kritaui)
install(TARGETS kritaedgedetection  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
