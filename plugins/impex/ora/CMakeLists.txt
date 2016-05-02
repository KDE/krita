include_directories( ${CMAKE_SOURCE_DIR}/krita/ui/ora ${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(libkritaconverter_LIB_SRCS
    ora_converter.cc
    )

set(kritaoraimport_SOURCES
    ora_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritaoraimport MODULE ${kritaoraimport_SOURCES})

target_link_libraries(kritaoraimport kritaui )

install(TARGETS kritaoraimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritaoraexport_SOURCES
    ora_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritaoraexport MODULE ${kritaoraexport_SOURCES})

target_link_libraries(kritaoraexport kritaui )

install(TARGETS kritaoraexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_ora.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

if(SHOULD_BUILD_FILEMANAGER_THUMBNAIL)
    install( FILES  krita_ora_thumbnail.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
endif()

