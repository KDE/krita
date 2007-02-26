include_directories(transformations)

set(kritadynamicbrushlib_LIB_SRCS
    kis_dynamic_shape.cc
    kis_dynamic_coloring.cc
    kis_dynamic_brush_registry.cc
    kis_dynamic_brush.cc
    transformations/kis_dynamic_transformation.cc
    transformations/kis_size_transformation.cc
    transformations/kis_darken_transformation.cc

)

kde4_automoc(${kritadynamicbrushlib_LIB_SRCS})

kde4_add_library(kritadynamicbrush SHARED ${kritadynamicbrushlib_LIB_SRCS})

target_link_libraries(kritadynamicbrush kritaui)

install(TARGETS kritadynamicbrush  DESTINATION ${LIB_INSTALL_DIR})

