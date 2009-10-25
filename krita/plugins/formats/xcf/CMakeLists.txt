add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    kis_xcf_converter.cc
    iccxcf.c
    )

set(kritaxcfimport_PART_SRCS
    kis_xcf_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritaxcfimport WITH_PREFIX ${kritaxcfimport_PART_SRCS})



target_link_libraries(kritaxcfimport kritaui xcf ${EXIV2_LIBRARIES} )

install(TARGETS kritaxcfimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritaxcfexport_PART_SRCS
    kis_xcf_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_ui_files(kritaxcfexport_PART_SRCS kis_wdg_options_xcf.ui )

kde4_add_plugin(kritaxcfexport WITH_PREFIX ${kritaxcfexport_PART_SRCS})



target_link_libraries(kritaxcfexport kritaui xcf ${EXIV2_LIBRARIES} )

install(TARGETS kritaxcfexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_xcf_import.desktop krita_xcf_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_xcf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
