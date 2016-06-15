set(kritaresourcemanager_SOURCES
    resourcemanager.cpp
    dlg_create_bundle.cpp
    dlg_bundle_manager.cpp
)

ki18n_wrap_ui(kritaresourcemanager_SOURCES
    wdgdlgcreatebundle.ui 
    wdgdlgbundlemanager.ui
)

add_library(kritaresourcemanager MODULE ${kritaresourcemanager_SOURCES})
target_link_libraries(kritaresourcemanager kritawidgets kritaui kritalibpaintop)
install(TARGETS kritaresourcemanager  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
