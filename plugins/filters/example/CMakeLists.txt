set(kritaexample_SOURCES example.cpp )
kis_add_library(kritaexample MODULE ${kritaexample_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritaexample kritaui)
install(TARGETS kritaexample  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
