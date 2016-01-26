set(kritaimagedocker_SOURCES
    imagedocker.cpp
    imagedocker_dock.cpp
    image_strip_scene.cpp
    image_view.cpp
)

ki18n_wrap_ui(kritaimagedocker_SOURCES forms/wdgimagedocker.ui forms/wdgImageViewPopup.ui)
add_library(kritaimagedocker MODULE ${kritaimagedocker_SOURCES})
target_link_libraries(kritaimagedocker kritaui)
install(TARGETS kritaimagedocker DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
