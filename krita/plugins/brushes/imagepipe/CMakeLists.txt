set(kritaimagepipebrush_PART_SRCS
    imagepipebrush.cpp
    kis_imagepipe_brush.cc
    kis_custom_brus_widget.cpp
    )
kde4_add_ui_files(kritaimagepipebrush_PART_SRCS
    wdgcustombrush.ui
)

kde4_add_plugin(kritaimagepipebrush ${kritaimagepipebrush_PART_SRCS})
target_link_libraries(kritaimagepipebrush kritaui)
install( TARGETS kritaimagepipebrush  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritaimagepipebrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

