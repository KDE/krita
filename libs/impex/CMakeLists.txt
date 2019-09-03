set(kritaimpex_LIB_SRCS
    KisExportCheckBase.cpp
    KisPreExportChecker.cpp
    KisExportCheckRegistry.cpp
)

kis_add_library(kritaimpex SHARED ${kritaimpex_HEADERS_MOC} ${kritaimpex_LIB_SRCS} )

generate_export_header(kritaimpex BASE_NAME kritaimpex)

target_link_libraries(kritaimpex KF5::CoreAddons KF5::I18n kritaimage)

set_target_properties(kritaimpex
    PROPERTIES VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritaimpex  ${INSTALL_TARGETS_DEFAULT_ARGS})
