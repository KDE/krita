
set(kritaimagedocker_PART_SRCS
    imagedocker.cpp
    imagedocker_dock.cpp
    image_strip_scene.cpp
    image_view.cpp
)

ki18n_wrap_ui(kritaimagedocker_PART_SRCS forms/wdgimagedocker.ui forms/wdgImageViewPopup.ui)

add_library(kritaimagedocker MODULE ${kritaimagedocker_PART_SRCS})
target_link_libraries(kritaimagedocker kritaui)

install(TARGETS kritaimagedocker DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kritaimagedocker.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
