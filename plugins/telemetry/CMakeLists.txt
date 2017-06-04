set(kritatelemetry_SOURCES
    kis_telemetry_provider.cpp
    kis_telemetry.cpp
    )

add_library(kritatelemetry MODULE ${kritatelemetry_SOURCES})
target_link_libraries(kritatelemetry UserFeedbackCore kritaui)
install(TARGETS kritatelemetry  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


