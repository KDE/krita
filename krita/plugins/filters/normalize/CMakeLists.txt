set(kritanormalize_PART_SRCS
    kis_normalize.cpp
)


add_library(kritanormalize MODULE ${kritanormalize_PART_SRCS})
kcoreaddons_desktop_to_json(kritanormalize kritanormalize.desktop)


target_link_libraries(kritanormalize kritaui)

install(TARGETS kritanormalize  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritanormalize.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
