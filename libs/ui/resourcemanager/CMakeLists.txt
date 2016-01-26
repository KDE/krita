add_subdirectory( tests )

set(kritaresourcemanager_SOURCES
    resourcemanager.cpp
	resourcebundle_manifest.cpp
    resourcebundle.cpp
    dlg_create_bundle.cpp
    dlg_bundle_manager.cpp
)

ki18n_wrap_ui(kritaresourcemanager_SOURCES
    wdgdlgcreatebundle.ui 
    wdgdlgbundlemanager.ui
)

add_library(kritaresourcemanager MODULE ${kritaresourcemanager_SOURCES})
target_link_libraries(kritaresourcemanager kritawidgets kritaui kritalibpaintop)
install(TARGETS kritaresourcemanager  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install(FILES resourcemanager.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
