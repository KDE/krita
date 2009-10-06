include_directories(  )
set(kritatrianglecolorselector_PART_SRCS trianglecolorselector.cc kis_triangle_color_selector_dock.cc )


# kde4_add_ui_files(kritatrianglecolorselector_PART_SRCS myuifile.ui )

kde4_add_plugin(kritatrianglecolorselector ${kritatrianglecolorselector_PART_SRCS})

target_link_libraries(kritatrianglecolorselector kritaui )

install(TARGETS kritatrianglecolorselector  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  trianglecolorselector.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritatrianglecolorselector.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

