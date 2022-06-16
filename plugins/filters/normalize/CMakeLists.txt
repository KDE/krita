set(kritanormalize_SOURCES
    kis_normalize.cpp
)
kis_add_library(kritanormalize MODULE ${kritanormalize_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritanormalize kritaui)
install(TARGETS kritanormalize  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES
    normalize.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
