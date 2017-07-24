include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
)

set(kritalibtelemetry_LIB_SRCS
    kis_telemetry_abstract.cpp
    kis_telemetry_instance.cpp
    kis_telemetry_actions.cpp
)

add_library(kritalibtelemetry SHARED ${kritalibtelemetry_LIB_SRCS} )
generate_export_header(kritalibtelemetry BASE_NAME kritalibtelemetry)


target_link_libraries(kritalibtelemetry  KUserFeedbackCore kritaimage) #kritaimage

set_target_properties(kritalibtelemetry PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritalibtelemetry  ${INSTALL_TARGETS_DEFAULT_ARGS})

