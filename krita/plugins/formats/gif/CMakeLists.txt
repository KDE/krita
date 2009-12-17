
include_directories( ${KRITA_INCLUDES} )

set(libkritaconverter_LIB_SRCS
    gif_converter.cpp
    )

set(kritagifimport_PART_SRCS
    gif_import.cpp
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritagifimport WITH_PREFIX ${kritagifimport_PART_SRCS})



target_link_libraries(kritagifimport kritaui )

install(TARGETS kritagifimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritagifexport_PART_SRCS
    gif_export.cpp
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritagifexport WITH_PREFIX ${kritagifexport_PART_SRCS})


target_link_libraries(kritagifexport kritaui )

install(TARGETS kritagifexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_gif_import.desktop krita_gif_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_gif.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
