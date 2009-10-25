add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    kis_gif_converter.cc
    iccgif.c
    )

set(kritagifimport_PART_SRCS
    kis_gif_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritagifimport WITH_PREFIX ${kritagifimport_PART_SRCS})



target_link_libraries(kritagifimport kritaui gif ${EXIV2_LIBRARIES} )

install(TARGETS kritagifimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritagifexport_PART_SRCS
    kis_gif_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_ui_files(kritagifexport_PART_SRCS kis_wdg_options_gif.ui )

kde4_add_plugin(kritagifexport WITH_PREFIX ${kritagifexport_PART_SRCS})



target_link_libraries(kritagifexport kritaui gif ${EXIV2_LIBRARIES} )

install(TARGETS kritagifexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_gif_import.desktop krita_gif_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_gif.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
