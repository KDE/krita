if (NOT MSVC AND NOT APPLE)
    add_subdirectory(tests)
endif()

configure_file(config_psd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_psd.h)

include_directories(
    ${CMAKE_BINARY_DIR}/libs/psd
    ${CMAKE_SOURCE_DIR}/libs/psd
)  #For kispsd_include.h

include_directories(SYSTEM
    ${ZLIB_INCLUDE_DIR}
)

set(LIB_PSD_SRCS 
    psd_colormode_block.cpp
    psd_image_data.cpp
)

#
# import
#

set(kritapsdimport_SOURCES
    psd_import.cc
    psd_loader.cpp
    ${LIB_PSD_SRCS}
)


add_library(kritapsdimport MODULE ${kritapsdimport_SOURCES})

target_link_libraries(kritapsdimport kritaglobal kritaui kritapsd KF5::I18n ${ZLIB_LIBRARIES})

install(TARGETS kritapsdimport DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

#
# export
#

set(kritapsdexport_SOURCES
    psd_export.cc
    psd_saver.cpp
    ${LIB_PSD_SRCS}
)

add_library(kritapsdexport MODULE ${kritapsdexport_SOURCES})

if (MSVC)
    target_link_libraries(kritapsdexport kritaui kritapsd kritaimpex ${WIN32_PLATFORM_NET_LIBS} ${ZLIB_LIBRARIES})
else ()	
    target_link_libraries(kritapsdexport kritaui kritapsd kritaimpex ${ZLIB_LIBRARIES})
endif ()	

install(TARGETS kritapsdexport DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_psd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
