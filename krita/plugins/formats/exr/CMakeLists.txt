
include_directories( ${OPENEXR_INCLUDE_DIR} ${KRITA_INCLUDES} )

add_definitions(${OPENEXR_DEFINITIONS})

set(libkritaconverter_LIB_SRCS
    exr_converter.cc
    )

set(kritaexrimport_PART_SRCS
    exr_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritaexrimport WITH_PREFIX ${kritaexrimport_PART_SRCS})



target_link_libraries(kritaexrimport kritaui ${OPENEXR_LIBRARIES} )

install(TARGETS kritaexrimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritaexrexport_PART_SRCS
    exr_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritaexrexport WITH_PREFIX ${kritaexrexport_PART_SRCS})


target_link_libraries(kritaexrexport kritaui ${OPENEXR_LIBRARIES} )

install(TARGETS kritaexrexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES
    exrchannels.schema
    DESTINATION ${DATA_INSTALL_DIR}/krita/metadata/schemas)

install( FILES  krita_exr_import.desktop krita_exr_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_exr.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
