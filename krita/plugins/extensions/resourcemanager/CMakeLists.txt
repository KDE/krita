add_subdirectory( tests )

set(kritaresourcemanager_PART_SRCS
    resourcemanager.cpp
	resourcebundle_manifest.cpp
    resourcebundle.cpp
    dlg_create_bundle.cpp
    dlg_bundle_manager.cpp
)

kde4_add_ui_files(kritaresourcemanager_PART_SRCS
    wdgdlgcreatebundle.ui 
    wdgdlgbundlemanager.ui
)

kde4_add_plugin(kritaresourcemanager ${kritaresourcemanager_PART_SRCS})

target_link_libraries(kritaresourcemanager kowidgets kritaui kritalibpaintop)

install(TARGETS kritaresourcemanager  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  resourcemanager.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaresourcemanager.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

