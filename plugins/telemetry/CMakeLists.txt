
set(kritaplugintelemetry_SOURCES
    telemetry_provider.cpp
    telemetry.cpp
    cpu_info_source.cpp
    tools_info_source.cpp
    assert_info_source.cpp
    image_properties_source.cpp
    actions_info_source.cpp
    general_info_source.cpp
    cpu_info.cpp
)
add_subdirectory( tests )

add_library(kritaplugintelemetry MODULE ${kritaplugintelemetry_SOURCES})
generate_export_header(kritaplugintelemetry EXPORT_MACRO_NAME TELEMETRY_PLUGIN_EXPORT)

target_link_libraries(kritaplugintelemetry kritalibtelemetry  KUserFeedbackCore kritaui)
install(TARGETS kritaplugintelemetry  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


