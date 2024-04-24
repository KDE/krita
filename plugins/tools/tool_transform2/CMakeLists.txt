add_subdirectory(tests)

set(kritatooltransform_static_SRCS
    tool_transform_args.cc
    kis_transform_mask_adapter.cpp
    KisAnimatedTransformMaskParamsHolder.cpp
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
    kis_mesh_transform_strategy.cpp
    kis_free_transform_strategy.cpp
    kis_free_transform_strategy_gsl_helpers.cpp
    kis_perspective_transform_strategy.cpp
    kis_transform_utils.cpp
    kis_convex_hull.cpp
    strokes/transform_stroke_strategy.cpp
    strokes/inplace_transform_stroke_strategy.cpp
    )

ki18n_wrap_ui(kritatooltransform_static_SRCS wdg_tool_transform.ui)

kis_add_library(kritatooltransform_static STATIC ${kritatooltransform_static_SRCS})
generate_export_header(kritatooltransform_static BASE_NAME kritatooltransform)

set(kritatooltransform_SOURCES
  tool_transform.cc
)

qt5_add_resources(kritatooltransform_SOURCES tool_transform.qrc)
kis_add_library(kritatooltransform MODULE ${kritatooltransform_SOURCES})

target_link_libraries(kritatooltransform_static
  PUBLIC
    kritaui
)

if (NOT GSL_FOUND)
  message (WARNING "KRITA WARNING! No GNU Scientific Library was found! Krita's Transform Tool will not be able to scale the image with handles. Please install GSL library.")
else ()
  target_link_libraries(kritatooltransform_static
    PUBLIC
      GSL::gsl
  )
endif ()

target_link_libraries(kritatooltransform PRIVATE kritatooltransform_static)

install(TARGETS kritatooltransform  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


install( FILES
    KisToolTransform.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
