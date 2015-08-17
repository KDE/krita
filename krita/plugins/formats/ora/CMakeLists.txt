
include_directories( ${CMAKE_SOURCE_DIR}/krita/ui/ora ${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(libkritaconverter_LIB_SRCS
    ora_converter.cc
    )

set(kritaoraimport_PART_SRCS
    ora_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritaoraimport ${kritaoraimport_PART_SRCS})

target_link_libraries(kritaoraimport kritaui ${KDE4_KIO_LIBS})

install(TARGETS kritaoraimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritaoraexport_PART_SRCS
    ora_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritaoraexport ${kritaoraexport_PART_SRCS})


target_link_libraries(kritaoraexport kritaui ${KDE4_KIO_LIBS})

install(TARGETS kritaoraexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_ora_import.desktop krita_ora_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_ora.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

if(SHOULD_BUILD_FILEMANAGER_THUMBNAIL)
    install( FILES  krita_ora_thumbnail.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
endif()


install(FILES krita_ora.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
