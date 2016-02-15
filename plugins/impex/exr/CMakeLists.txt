add_subdirectory(tests)

include_directories( ${OPENEXR_INCLUDE_DIR} )

add_definitions(${OPENEXR_DEFINITIONS})

set(libkritaconverter_LIB_SRCS
    exr_converter.cc
    kis_exr_layers_sorter.cpp
    exr_extra_tags.cpp
    )

set(kritaexrimport_SOURCES
    exr_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritaexrimport MODULE ${kritaexrimport_SOURCES})

target_link_libraries(kritaexrimport kritaui  ${OPENEXR_LIBRARIES} )

install(TARGETS kritaexrimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

set(kritaexrexport_SOURCES
    exr_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

ki18n_wrap_ui(kritaexrexport_SOURCES exr_export_widget.ui )

add_library(kritaexrexport MODULE ${kritaexrexport_SOURCES})

target_link_libraries(kritaexrexport kritaui  ${OPENEXR_LIBRARIES} )

install(TARGETS kritaexrexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES
    exrchannels.schema
    DESTINATION ${DATA_INSTALL_DIR}/krita/metadata/schemas)

install( PROGRAMS  krita_exr.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
