set(kritacustombrush_PART_SRCS
    custombrush.cpp
    kis_custom_brush.cc
    )
kde4_add_plugin(kritacustombrush ${kritacustombrush_PART_SRCS})
target_link_libraries(kritacustombrush kritaui)
install( TARGETS kritacustombrush  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritacustombrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

