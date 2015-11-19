add_subdirectory(tests)

configure_file(config_psd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_psd.h)

include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR}/krita/libpsd)  #For kispsd_include.h

set(LIB_PSD_SRCS 
    psd_header.cpp
    psd_colormode_block.cpp
    psd_resource_section.cpp
    psd_resource_block.cpp
    psd_layer_section.cpp
    psd_layer_record.cpp
    psd_image_data.cpp
    psd_pixel_utils.cpp
    psd_additional_layer_info_block.cpp
)

set(kritapsdimport_PART_SRCS
    psd_import.cc
    psd_loader.cpp
    ${LIB_PSD_SRCS}
)

add_library(kritapsdimport MODULE ${kritapsdimport_PART_SRCS})
kcoreaddons_desktop_to_json(kritapsdimport krita_psd_import.desktop)

if (WIN32)
	target_link_libraries(kritapsdimport kritaui kritapsd  ${WIN32_PLATFORM_NET_LIBS} ${ZLIB_LIBRARIES})
else ()
	target_link_libraries(kritapsdimport kritaui kritapsd  ${ZLIB_LIBRARIES})
endif ()

install(TARGETS kritapsdimport DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritapsdexport_PART_SRCS
    psd_export.cc
    psd_saver.cpp
    ${LIB_PSD_SRCS}
)

add_library(kritapsdexport MODULE ${kritapsdexport_PART_SRCS})
kcoreaddons_desktop_to_json(kritapsdexport krita_psd_export.desktop)

if (WIN32)
    target_link_libraries(kritapsdexport kritaui ${WIN32_PLATFORM_NET_LIBS} ${ZLIB_LIBRARIES})
else ()	
    target_link_libraries(kritapsdexport kritaui ${ZLIB_LIBRARIES})
endif ()	
install(TARGETS kritapsdexport DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_psd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
