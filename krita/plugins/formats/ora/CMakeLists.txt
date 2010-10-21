
include_directories( ${KRITA_INCLUDES} ${CMAKE_SOURCE_DIR}/krita/ui/openrastertk ${CMAKE_SOURCE_DIR}/krita/image/metadata)

add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(libkritaconverter_LIB_SRCS
    ora_converter.cc
    ora_load_context.cc
    ora_save_context.cc
    )

set(kritaoraimport_PART_SRCS
    ora_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritaoraimport WITH_PREFIX ${kritaoraimport_PART_SRCS})

target_link_libraries(kritaoraimport kritaui )

install(TARGETS kritaoraimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritaoraexport_PART_SRCS
    ora_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritaoraexport WITH_PREFIX ${kritaoraexport_PART_SRCS})


target_link_libraries(kritaoraexport kritaui )

install(TARGETS kritaoraexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_ora_import.desktop krita_ora_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_ora.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

find_package(SharedMimeInfo REQUIRED)
macro_log_feature(SHARED_MIME_INFO_FOUND "SharedMimeInfo" "Shared Mime Info" "http://freedesktop.org/wiki/Specifications/shared-mime-info-spec" TRUE "" "Required by the OpenRaster file format")

install(FILES krita_ora.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
