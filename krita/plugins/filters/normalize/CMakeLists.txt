set(kritanormalize_SOURCES
    kis_normalize.cpp
)


add_library(kritanormalize MODULE ${kritanormalize_SOURCES})
kcoreaddons_desktop_to_json(kritanormalize kritanormalize.desktop)


target_link_libraries(kritanormalize kritaui)

install(TARGETS kritanormalize  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
