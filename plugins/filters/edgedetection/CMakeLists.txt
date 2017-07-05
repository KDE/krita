set(kritaedgedetection_SOURCES
    kis_edge_detection_filter.cpp
)
add_library(kritaedgedetection MODULE ${kritaedgedetection_SOURCES})
target_link_libraries(kritaedgedetection kritaui)
install(TARGETS kritaedgedetection  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
