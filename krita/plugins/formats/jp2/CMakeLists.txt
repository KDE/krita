
include_directories( ${KRITA_INCLUDES}  ${OPENJPEG_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    jp2_converter.cc
    )

set(kritajp2import_PART_SRCS
    jp2_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritajp2import WITH_PREFIX ${kritajp2import_PART_SRCS})



target_link_libraries(kritajp2import kritaui ${OPENJPEG_LIBRARIES} )

install(TARGETS kritajp2import  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritajp2export_PART_SRCS
    jp2_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_ui_files(kritajp2export_PART_SRCS kis_wdg_options_jp2.ui )

kde4_add_plugin(kritajp2export WITH_PREFIX ${kritajp2export_PART_SRCS})


target_link_libraries(kritajp2export kritaui ${OPENJPEG_LIBRARIES} )

install(TARGETS kritajp2export  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_jp2_import.desktop krita_jp2_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_jp2.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
