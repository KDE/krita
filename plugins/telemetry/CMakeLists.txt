set(kritatelemetry_SOURCES
    kis_telemetry_provider.cpp
    )

add_library(kritatelemetry SHARED ${kritatelemetry_SOURCES})

target_link_libraries(kritatelemetry kritaui)
install(TARGETS kritatelemetry  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


