set(kritapsd_SRCS
    psd_additional_layer_info_block.cpp
    psd_header.cpp
    psd_layer_record.cpp
    psd_layer_section.cpp
    psd_pixel_utils.cpp
    psd_resource_block.cpp
    psd_resource_section.cpp
)

add_library(kritapsd SHARED ${kritapsd_SRCS})
generate_export_header(kritapsd)

target_link_libraries(kritapsd
    PUBLIC
        kritaimage
        kritapsdutils
)

set_target_properties(kritapsd PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION}
    SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritapsd ${INSTALL_TARGETS_DEFAULT_ARGS})
