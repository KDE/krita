set( extensions_plugin_PART_SRCS 
extensions_plugin.cc 
kis_hsv_adjustment.cpp
kis_dodgehighlights_adjustment.cpp
kis_dodgemidtones_adjustment.cpp
kis_dodgeshadows_adjustment.cpp
kis_burnhighlights_adjustment.cpp
kis_burnmidtones_adjustment.cpp
kis_burnshadows_adjustment.cpp
kis_color_balance_midtones_adjustment.cpp
kis_color_balance_shadows_adjustment.cpp
kis_color_balance_highlights_adjustment.cpp
)

kde4_add_plugin( krita_colorspaces_extensions ${extensions_plugin_PART_SRCS} )

target_link_libraries(krita_colorspaces_extensions pigmentcms ${OPENEXR_LIBRARIES})

install( TARGETS krita_colorspaces_extensions DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES krita_colorspaces_extensions_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
