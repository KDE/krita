
include_directories( ${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(kritaqmlexport_PART_SRCS
    qml_converter.cc
    qml_export.cc
    )

add_library(kritaqmlexport MODULE ${kritaqmlexport_PART_SRCS})
kcoreaddons_desktop_to_json(kritaqmlexport krita_qml_export.desktop)

target_link_libraries(kritaqmlexport kritaui )

install(TARGETS kritaqmlexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
