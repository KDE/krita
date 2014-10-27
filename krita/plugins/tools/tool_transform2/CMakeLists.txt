set(kritatooltransform_PART_SRCS
    tool_transform.cc
    tool_transform_args.cc
    kis_transform_mask_adapter.cpp
    tool_transform_changes_tracker.cpp
    kis_tool_transform.cc
    kis_tool_transform_config_widget.cpp
    kis_transform_strategy_base.cpp
    kis_warp_transform_strategy.cpp
    kis_cage_transform_strategy.cpp
    kis_simplified_action_policy_strategy.cpp
    kis_liquify_transform_strategy.cpp
    kis_liquify_paint_helper.cpp
    kis_liquify_paintop.cpp
    kis_liquify_properties.cpp
    kis_free_transform_strategy.cpp
    kis_free_transform_strategy_gsl_helpers.cpp
    kis_perspective_transform_strategy.cpp
    kis_transform_utils.cpp
    strokes/transform_stroke_strategy.cpp
    )


kde4_add_ui_files(kritatooltransform_PART_SRCS wdg_tool_transform.ui)

kde4_add_plugin(kritatooltransform ${kritatooltransform_PART_SRCS})

if (NOT GSL_FOUND)
  message (WARNING "KRITA WARNING! No GNU Scientific Library was found! Krita's Transform Tool will not be able to scale the image with handles. Please install GSL library.")
  target_link_libraries(kritatooltransform kritaui)
else (NOT GSL_FOUND)
  target_link_libraries(kritatooltransform kritaui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
endif (NOT GSL_FOUND)

install(TARGETS kritatooltransform  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  rotate_cursor.xpm
                krita_tool_transform.png
                dark_krita_tool_transform.png
                light_krita_tool_transform.png
                krita_tool_transform_recursive.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatooltransform.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
