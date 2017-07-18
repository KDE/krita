
set(kritatelemetry_SOURCES
    kis_telemetry_provider.cpp
    kis_telemetry.cpp
    kis_cpuinfosource.cpp
    kis_toolsinfosource.cpp
    kis_tickets.cpp
    kis_assertinfosource.cpp
)
add_subdirectory( tests )

add_library(kritatelemetry MODULE ${kritatelemetry_SOURCES})
generate_export_header(kritatelemetry EXPORT_MACRO_NAME TELEMETRY_EXPORT)

target_link_libraries(kritatelemetry KUserFeedbackCore kritaui)
install(TARGETS kritatelemetry  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


