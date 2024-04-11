add_subdirectory(tests)

set(kritargbeimport_SOURCES
    RGBEImport.cpp
)

add_library(kritargbeimport MODULE ${kritargbeimport_SOURCES})
target_link_libraries(kritargbeimport kritaui)

install(TARGETS kritargbeimport DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritargbeexport_SOURCES
    RGBEExport.cpp
    kis_wdg_options_rgbe.cpp
)

ki18n_wrap_ui(kritargbeexport_SOURCES kis_wdg_options_rgbe.ui )

kis_add_library(kritargbeexport MODULE ${kritargbeexport_SOURCES})

target_link_libraries(kritargbeexport kritaui kritalibkra kritaimpex)

install(TARGETS kritargbeexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install(PROGRAMS krita_rgbe.desktop DESTINATION ${KDE_INSTALL_APPDIR})
