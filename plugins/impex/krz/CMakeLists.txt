set(kritakrzexport_SOURCES
    krz_export.cpp
)

kis_add_library(kritakrzexport MODULE ${kritakrzexport_SOURCES})

target_link_libraries(kritakrzexport kritaui kritalibkra kritaimpex)

install(TARGETS kritakrzexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install(FILES krita_krz.desktop DESTINATION ${KDE_INSTALL_APPDIR})
