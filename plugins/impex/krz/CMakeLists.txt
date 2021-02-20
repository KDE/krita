set(kritakrzexport_SOURCES
    krz_export.cpp
)

add_library(kritakrzexport MODULE ${kritakrzexport_SOURCES})

target_link_libraries(kritakrzexport kritaui kritalibkra kritaimpex)

install(TARGETS kritakrzexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_krz.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
