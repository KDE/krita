set(kritatextbrush_PART_SRCS
    textbrush.cpp
    kis_text_brush.cc
    )
kde4_add_plugin(kritatextbrush ${kritatextbrush_PART_SRCS})
target_link_libraries(kritatextbrush kritaui)
install( TARGETS kritatextbrush  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritatextbrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

