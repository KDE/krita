set(kritaresourcemanager_SOURCES
    resourcemanager.cpp
    dlg_create_bundle.cpp
    dlg_bundle_manager.cpp
    dlg_embed_tags.cpp
    DlgResourceManager.cpp
)

ki18n_wrap_ui(kritaresourcemanager_SOURCES
    wdgdlgcreatebundle.ui 
    wdgdlgbundlemanager.ui
    wdgdlgembedtags.ui
    WdgDlgResourceManager.ui
)

add_library(kritaresourcemanager MODULE ${kritaresourcemanager_SOURCES})
target_link_libraries(kritaresourcemanager kritawidgets kritaui kritalibpaintop kritaresources kritaresourcewidgets)
install(TARGETS kritaresourcemanager  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(FILES resourcemanager.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
