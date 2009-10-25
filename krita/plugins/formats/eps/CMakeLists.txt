add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    kis_eps_converter.cc
    icceps.c
    )

set(kritaepsimport_PART_SRCS
    kis_eps_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritaepsimport WITH_PREFIX ${kritaepsimport_PART_SRCS})

target_link_libraries(kritaepsimport kritaui eps ${EXIV2_LIBRARIES} )

install(TARGETS kritaepsimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritaepsexport_PART_SRCS
    kis_eps_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_ui_files(kritaepsexport_PART_SRCS kis_wdg_options_eps.ui )

kde4_add_plugin(kritaepsexport WITH_PREFIX ${kritaepsexport_PART_SRCS})

target_link_libraries(kritaepsexport kritaui eps ${EXIV2_LIBRARIES} )

install(TARGETS kritaepsexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_eps_import.desktop krita_eps_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_eps.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
