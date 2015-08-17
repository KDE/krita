

set(kritanormalize_PART_SRCS kis_normalize.cpp )

kde4_add_plugin(kritanormalize ${kritanormalize_PART_SRCS})



target_link_libraries(kritanormalize kritaui)

install(TARGETS kritanormalize  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritanormalize.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


