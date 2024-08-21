set( extensions_plugin_SOURCES 
    extensions_plugin.cc
    kis_hsv_adjustment.cpp
    kis_dodgehighlights_adjustment.cpp
    kis_dodgemidtones_adjustment.cpp
    kis_dodgeshadows_adjustment.cpp
    kis_burnhighlights_adjustment.cpp
    kis_burnmidtones_adjustment.cpp
    kis_burnshadows_adjustment.cpp
    kis_color_balance_adjustment.cpp
    kis_desaturate_adjustment.cpp
)

kis_add_library(krita_colorspaces_extensions MODULE ${extensions_plugin_SOURCES})
target_link_libraries(krita_colorspaces_extensions kritapigment kritaglobal ${LINK_OPENEXR_LIB} KF${KF_MAJOR}::I18n KF${KF_MAJOR}::CoreAddons)
install( TARGETS krita_colorspaces_extensions DESTINATION ${KRITA_PLUGIN_INSTALL_DIR} )
