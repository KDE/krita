add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    kis_psd_converter.cc
    iccpsd.c
    )

set(kritapsdimport_PART_SRCS
    kis_psd_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritapsdimport WITH_PREFIX ${kritapsdimport_PART_SRCS})

target_link_libraries(kritapsdimport kritaui psd ${EXIV2_LIBRARIES} )

install(TARGETS kritapsdimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritapsdexport_PART_SRCS
    kis_psd_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_ui_files(kritapsdexport_PART_SRCS kis_wdg_options_psd.ui )

kde4_add_plugin(kritapsdexport WITH_PREFIX ${kritapsdexport_PART_SRCS})



target_link_libraries(kritapsdexport kritaui psd ${EXIV2_LIBRARIES} )

install(TARGETS kritapsdexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_psd_import.desktop krita_psd_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_psd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
