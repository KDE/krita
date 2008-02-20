set(kritaautobrush_PART_SRCS
    autobrush.cpp
    kis_autobrush.cc
    )
kde4_add_plugin(kritaautobrush ${kritaautobrush_PART_SRCS})
target_link_libraries(kritaautobrush kritaui)
install( TARGETS kritaautobrush  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritaautobrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

