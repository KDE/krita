set(kritaresourcemanager_SOURCES
    resourcemanager.cpp
    dlg_create_bundle.cpp
    dlg_bundle_manager.cpp
    dlg_embed_tags.cpp
    DlgResourceManager.cpp
    wdgtagselection.cpp
    ResourceImporter.cpp
    DlgResourceTypeForFile.cpp
    wdg_side.cpp
    page_resource_chooser.cpp
    page_tag_chooser.cpp
    page_bundle_saver.cpp
    page_metadata_info.cpp
    wdg_resource_preview.cpp
    KisResourceItemViewer.cpp
    wdgtagpreview.cpp
)

ki18n_wrap_ui(kritaresourcemanager_SOURCES
    wdgdlgcreatebundle.ui 
    wdgdlgbundlemanager.ui
    wdgdlgembedtags.ui
    WdgDlgResourceManager.ui
    wdgside.ui
    pageresourcechooser.ui
    pagetagchooser.ui
    pagebundlesaver.ui
    pagemetadatainfo.ui
    wdgresourcepreview.ui
    wdgtagpreview.ui
)

kis_add_library(kritaresourcemanager MODULE ${kritaresourcemanager_SOURCES})
target_link_libraries(kritaresourcemanager kritawidgets kritaui kritalibpaintop kritaresources kritaresourcewidgets)
install(TARGETS kritaresourcemanager  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(FILES resourcemanager.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
