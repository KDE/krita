
set(kritaextensionsmanager_PART_SRCS
    ExtensionsManager.cpp
    ExtensionsManagerPlugin.cc
    ExtensionsManagerWindow.cpp )


kde4_add_ui_files(kritaextensionsmanager_PART_SRCS
    ExtensionsManagerWidget.ui
    ExtensionInformationWidget.ui )

kde4_add_plugin(kritaextensionsmanager ${kritaextensionsmanager_PART_SRCS})

target_link_libraries(kritaextensionsmanager kritaui)

install(TARGETS kritaextensionsmanager  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  extensionsmanager.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaextensionsmanager.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

