set(kritametadata_LIB_SRCS
    kis_legacy_importer.cc
    kis_meta_data_entry.cc
    kis_meta_data_filter.cc
    kis_meta_data_filter_p.cc
    kis_meta_data_filter_registry.cc
    kis_meta_data_filter_registry_model.cc
    kis_meta_data_io_backend.cc
    kis_meta_data_merge_strategy.cc
    kis_meta_data_merge_strategy_p.cc
    kis_meta_data_merge_strategy_registry.cc
    kis_meta_data_parser.cc
    kis_meta_data_schema.cc
    kis_meta_data_schema_registry.cc
    kis_meta_data_store.cc
    kis_meta_data_type_info.cc
    kis_meta_data_validator.cc
    kis_meta_data_value.cc
)

add_library(kritametadata SHARED ${kritametadata_LIB_SRCS} )
generate_export_header(kritametadata)

target_link_libraries(kritametadata kritaversion kritaglobal kritaplugin kritawidgetutils)

set_target_properties(kritametadata PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritametadata  ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
