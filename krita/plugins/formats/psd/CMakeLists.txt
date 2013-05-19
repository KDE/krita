add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} )

macro_optional_find_package(ZLIB)
macro_bool_to_01(ZLIB_FOUND HAVE_ZLIB)
configure_file(config_psd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_psd.h)


set(LIB_PSD_SRCS 
    psd.cpp
    psd_utils.cpp
    psd_header.cpp
    psd_colormode_block.cpp
    psd_resource_section.cpp
    psd_resource_block.cpp
    psd_layer_section.cpp
    psd_layer_record.cpp
    psd_image_data.cpp
    compression.cpp
)

set(kritapsdimport_PART_SRCS
    psd_import.cc
    psd_loader.cpp
    ${LIB_PSD_SRCS}
)

kde4_add_plugin(kritapsdimport ${kritapsdimport_PART_SRCS})
if (WIN32)
	target_link_libraries(kritapsdimport kritaui ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
	target_link_libraries(kritapsdimport kritaui )
endif (WIN32)

install(TARGETS kritapsdimport DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritapsdexport_PART_SRCS
    psd_export.cc
    psd_saver.cpp
    ${LIB_PSD_SRCS}
)

kde4_add_plugin(kritapsdexport ${kritapsdexport_PART_SRCS})

if (WIN32)
    target_link_libraries(kritapsdexport kritaui ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)	
    target_link_libraries(kritapsdexport kritaui )
endif (WIN32)	
install(TARGETS kritapsdexport DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES krita_psd_import.desktop 
               krita_psd_export.desktop  
DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_psd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
