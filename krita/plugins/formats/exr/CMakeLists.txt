add_subdirectory(tests)

include_directories( ${OPENEXR_INCLUDE_DIR} )

add_definitions(${OPENEXR_DEFINITIONS})

set(libkritaconverter_LIB_SRCS
    exr_converter.cc
    kis_exr_layers_sorter.cpp
    exr_extra_tags.cpp
    )

set(kritaexrimport_PART_SRCS
    exr_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritaexrimport MODULE ${kritaexrimport_PART_SRCS})
kcoreaddons_desktop_to_json(kritaexrimport krita_exr_import.desktop)


target_link_libraries(kritaexrimport kritaui  ${OPENEXR_LIBRARIES} )

install(TARGETS kritaexrimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritaexrexport_PART_SRCS
    exr_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

ki18n_wrap_ui(kritaexrexport_PART_SRCS exr_export_widget.ui )

add_library(kritaexrexport MODULE ${kritaexrexport_PART_SRCS})
kcoreaddons_desktop_to_json(kritaexrexport krita_exr_export.desktop)

target_link_libraries(kritaexrexport kritaui  ${OPENEXR_LIBRARIES} )

install(TARGETS kritaexrexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES
    exrchannels.schema
    DESTINATION ${DATA_INSTALL_DIR}/krita/metadata/schemas)

install( FILES  krita_exr_import.desktop krita_exr_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_exr.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
