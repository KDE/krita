# export

set(kritaspriterexport_SOURCES
    kis_spriter_export.cpp
)

kis_add_library(kritaspriterexport MODULE ${kritaspriterexport_SOURCES})

target_link_libraries(kritaspriterexport kritaui kritaimpex)

install(TARGETS kritaspriterexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_spriter.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
