add_subdirectory( tests )

set(kritaresourcemanager_PART_SRCS
    resourcemanager.cpp
	resourcebundle_manifest.cpp
    resourcebundle.cpp
    dlg_create_bundle.cpp
    dlg_bundle_manager.cpp
)

ki18n_wrap_ui(kritaresourcemanager_PART_SRCS
    wdgdlgcreatebundle.ui 
    wdgdlgbundlemanager.ui
)

add_library(kritaresourcemanager MODULE ${kritaresourcemanager_PART_SRCS})
kcoreaddons_desktop_to_json(kritaresourcemanager kritaresourcemanager.desktop)

target_link_libraries(kritaresourcemanager kowidgets kritaui kritalibpaintop)

install(TARGETS kritaresourcemanager  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  resourcemanager.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaresourcemanager.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

