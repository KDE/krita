add_subdirectory(tests)

set(kritatooltransform_SOURCES
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

qt5_add_resources(kritatooltransform_SOURCES tool_transform.qrc)
ki18n_wrap_ui(kritatooltransform_SOURCES wdg_tool_transform.ui)

add_library(kritatooltransform MODULE ${kritatooltransform_SOURCES})
generate_export_header(kritatooltransform BASE_NAME kritatooltransform)

if (NOT GSL_FOUND)
  message (WARNING "KRITA WARNING! No GNU Scientific Library was found! Krita's Transform Tool will not be able to scale the image with handles. Please install GSL library.")
  target_link_libraries(kritatooltransform kritaui)
else ()
  target_link_libraries(kritatooltransform kritaui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
endif ()

install(TARGETS kritatooltransform  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


install( FILES
    KisToolTransform.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
