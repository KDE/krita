set(libkritaconverter_LIB_SRCS
    kra_converter.cpp
    kis_colorize_dom_utils.cpp
    kis_kra_load_visitor.cpp
    kis_kra_loader.cpp
    kis_kra_save_visitor.cpp
    kis_kra_saver.cpp
    kis_kra_savexml_visitor.cpp
    kis_kra_utils.cpp
    )

set(kritakraimport_SOURCES
    kra_import.cpp
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritakraimport MODULE ${kritakraimport_SOURCES})

target_link_libraries(kritakraimport kritaui )

install(TARGETS kritakraimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritakraexport_SOURCES
    kra_export.cpp
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritakraexport MODULE ${kritakraexport_SOURCES})

target_link_libraries(kritakraexport kritaui )

install(TARGETS kritakraexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_kra.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

if(SHOULD_BUILD_FILEMANAGER_THUMBNAIL)
    install( FILES  krita_kra_thumbnail.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
endif()

