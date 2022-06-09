set(kritaposterize_SOURCES posterize.cpp )
kis_add_library(kritaposterize MODULE ${kritaposterize_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritaposterize kritaui)
install(TARGETS kritaposterize  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
