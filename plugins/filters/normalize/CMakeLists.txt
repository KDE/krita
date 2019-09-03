set(kritanormalize_SOURCES
    kis_normalize.cpp
)
kis_add_library(kritanormalize MODULE ${kritanormalize_SOURCES})
target_link_libraries(kritanormalize kritaui)
install(TARGETS kritanormalize  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
