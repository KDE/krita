

set(kritatoolperspectivetransform_PART_SRCS
    tool_perspectivetransform.cc
    kis_tool_perspectivetransform.cc
    kis_perspectivetransform_worker.cpp
    )

kde4_add_ui_files(kritatoolperspectivetransform_PART_SRCS wdgperspectivetransform.ui)

kde4_add_plugin(kritatoolperspectivetransform ${kritatoolperspectivetransform_PART_SRCS})

target_link_libraries(kritatoolperspectivetransform kritaui)

install(TARGETS kritatoolperspectivetransform  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_perspectivetransform.png tool_perspectivetransform_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolperspectivetransform.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


