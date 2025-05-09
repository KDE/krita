add_subdirectory(tests)

set(libkritaconverter_LIB_SRCS
    exr_converter.cc
    kis_exr_layers_sorter.cpp
    exr_extra_tags.cpp
    )

set(kritaexrimport_SOURCES
    exr_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kis_add_library(kritaexrimport MODULE ${kritaexrimport_SOURCES})

target_link_libraries(kritaexrimport kritaui kritalibkra OpenEXR::IlmImf)

install(TARGETS kritaexrimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritaexrexport_SOURCES
    exr_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

ki18n_wrap_ui(kritaexrexport_SOURCES exr_export_widget.ui )

kis_add_library(kritaexrexport MODULE ${kritaexrexport_SOURCES})

target_link_libraries(kritaexrexport kritaui kritalibkra kritaimpex OpenEXR::IlmImf)

install(TARGETS kritaexrexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES
    exrchannels.schema
    DESTINATION ${KDE_INSTALL_DATADIR}/krita/metadata/schemas)

install(FILES krita_exr.desktop DESTINATION ${KDE_INSTALL_APPDIR})
