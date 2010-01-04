add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} )

macro_optional_find_package(ZLIB)
macro_bool_to_01(ZLIB_FOUND HAVE_ZLIB)
configure_file(config_psd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_psd.h)


set(LIB_PSD_SRCS 
    psd_utils.cpp
    psd_header.cpp
    psd_colormode_block.cpp
    psd_resource_section.cpp
    psd_resource_block.cpp
    psd_layer_section.cpp
    psd_layer_record.cpp
)

set(kritapsdimport_PART_SRCS
    psd_import.cc
    psd_loader.cpp
    ${LIB_PSD_SRCS}
)

kde4_add_plugin(kritapsdimport WITH_PREFIX ${kritapsdimport_PART_SRCS})
target_link_libraries(kritapsdimport kritaui )
install(TARGETS kritapsdimport DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritapsdexport_PART_SRCS
    psd_export.cc
    psd_saver.cpp
    ${LIB_PSD_SRCS}
)

kde4_add_plugin(kritapsdexport WITH_PREFIX ${kritapsdexport_PART_SRCS})
target_link_libraries(kritapsdexport kritaui )
install(TARGETS kritapsdexport DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  krita_psd_import.desktop krita_psd_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_psd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
