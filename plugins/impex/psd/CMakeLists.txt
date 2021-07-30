if (NOT MSVC AND NOT APPLE)
    add_subdirectory(tests)
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/libs/psd
    ${CMAKE_SOURCE_DIR}/libs/psd
)  #For kispsd_include.h

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

target_link_libraries(kritapsdimport kritaglobal kritaui kritapsd KF5::I18n)

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
    target_link_libraries(kritapsdexport kritaui kritapsd kritaimpex kritastore ${WIN32_PLATFORM_NET_LIBS})
else ()
    target_link_libraries(kritapsdexport kritaui kritapsd kritaimpex)
endif ()

install(TARGETS kritapsdexport DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_psd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
