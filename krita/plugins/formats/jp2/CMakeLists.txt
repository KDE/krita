
include_directories( ${OPENJPEG_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    jp2_converter.cc
    )

set(kritajp2import_PART_SRCS
    jp2_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritajp2import MODULE ${kritajp2import_PART_SRCS})
kcoreaddons_desktop_to_json(kritajp2import krita_jp2_import.desktop)


target_link_libraries(kritajp2import kritaui  ${OPENJPEG_LIBRARIES} )

install(TARGETS kritajp2import  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritajp2export_PART_SRCS
    jp2_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

ki18n_wrap_ui(kritajp2export_PART_SRCS kis_wdg_options_jp2.ui )

add_library(kritajp2export MODULE ${kritajp2export_PART_SRCS})
kcoreaddons_desktop_to_json(kritajp2export krita_jp2_export.desktop)


target_link_libraries(kritajp2export kritaui  ${OPENJPEG_LIBRARIES} )

install(TARGETS kritajp2export  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_jp2_import.desktop krita_jp2_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_jp2.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
