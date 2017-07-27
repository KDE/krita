
set(kritaplugintelemetry_SOURCES
    kis_telemetry_provider.cpp
    kis_telemetry.cpp
    kis_cpuinfosource.cpp
    kis_toolsinfosource.cpp
    kis_assertinfosource.cpp
    kis_imagepropertiessource.cpp
    kis_actionsinfosource.cpp
)
add_subdirectory( tests )

add_library(kritaplugintelemetry MODULE ${kritaplugintelemetry_SOURCES})
generate_export_header(kritaplugintelemetry EXPORT_MACRO_NAME TELEMETRY_PLUGIN_EXPORT)

target_link_libraries(kritaplugintelemetry kritalibtelemetry  KUserFeedbackCore kritaui)
install(TARGETS kritaplugintelemetry  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


