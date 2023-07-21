set(kritaresettransparent_SOURCES KisResetTransparentFilter.cpp )
kis_add_library(kritaresettransparent MODULE ${kritaresettransparent_SOURCES})
target_link_libraries(kritaresettransparent kritaui)
install(TARGETS kritaresettransparent  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
