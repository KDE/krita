
set(kritaimagedocker_PART_SRCS
    imagedocker.cpp
    imagedocker_dock.cpp
    kis_image_strip_scene.cpp
    kis_image_view.cpp
)

kde4_add_ui_files(kritaimagedocker_PART_SRCS forms/wdgimagedocker.ui forms/wdgImageViewPopup.ui)

kde4_add_plugin(kritaimagedocker ${kritaimagedocker_PART_SRCS})
target_link_libraries(kritaimagedocker kritaui)

install(TARGETS kritaimagedocker DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kritaimagedocker.desktop DESTINATION ${SERVICES_INSTALL_DIR})
