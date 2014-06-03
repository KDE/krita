
include_directories( ${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(kritaqmlexport_PART_SRCS
    qml_converter.cc
    qml_export.cc
    )

kde4_add_plugin(kritaqmlexport ${kritaqmlexport_PART_SRCS})

target_link_libraries(kritaqmlexport kritaui )

install(TARGETS kritaqmlexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_qml_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
