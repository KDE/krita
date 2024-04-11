add_subdirectory( tests )

set(kritalibbrush_LIB_SRCS
    kis_predefined_brush_factory.cpp
    kis_auto_brush.cpp
    kis_boundary.cc
    kis_brush.cpp
    kis_scaling_size_brush.cpp
    kis_brush_registry.cpp
    KisBrushServerProvider.cpp
    kis_gbr_brush.cpp
    kis_abr_brush.cpp
    kis_abr_brush_collection.cpp
    kis_imagepipe_brush.cpp
    kis_pipebrush_parasite.cpp
    kis_png_brush.cpp
    kis_svg_brush.cpp
    kis_qimage_pyramid.cpp
    kis_text_brush.cpp
    kis_auto_brush_factory.cpp
    kis_text_brush_factory.cpp
    KisAbrStorage.cpp
    KisColorfulBrush.cpp
    KisBrushTypeMetaDataFixup.cpp
    KisBrushModel.cpp
)

kis_add_library(kritalibbrush SHARED ${kritalibbrush_LIB_SRCS})
generate_export_header(kritalibbrush BASE_NAME kritabrush EXPORT_MACRO_NAME BRUSH_EXPORT)

target_link_libraries(kritalibbrush kritaimage Qt6::Svg kritamultiarch lager)

set_target_properties(kritalibbrush PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritalibbrush  ${INSTALL_TARGETS_DEFAULT_ARGS})

