add_subdirectory(tests)

# Static lib for dock (used by plugin and tests)
kis_add_library(kritacomfyuiremote_static STATIC ComfyUIRemoteDock.cpp)
target_link_libraries(kritacomfyuiremote_static kritaimage kritaui)

# Plugin module
set(kritacomfyuiremote_SOURCES ComfyUIRemotePlugin.cpp)
kis_add_library(kritacomfyuiremote MODULE ${kritacomfyuiremote_SOURCES})
target_link_libraries(kritacomfyuiremote kritacomfyuiremote_static)
install(TARGETS kritacomfyuiremote DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
