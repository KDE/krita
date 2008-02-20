set(kritaimagepipebrush_PART_SRCS
    imagepipebrush.cpp
    kis_imagepipe_brush.cc
    )
kde4_add_plugin(kritaimagepipebrush ${kritaimagepipebrush_PART_SRCS})
target_link_libraries(kritaimagepipebrush kritaui)
install( TARGETS kritaimagepipebrush  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritaimagepipebrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

